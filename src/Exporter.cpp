/**
 * \file Exporter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Exporter.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "Converter.h"
#include "Format.h"
#include "gettext.h"
#include "LyXRC.h"
#include "Mover.h"
#include "output_plaintext.h"
#include "OutputParams.h"
#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/Package.h"

#include <boost/filesystem/operations.hpp>

using std::find;
using std::string;
using std::vector;


namespace lyx {

using support::addName;
using support::bformat;
using support::changeExtension;
using support::contains;
using support::doesFileExist;
using support::FileName;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::prefixIs;

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;

namespace {

vector<string> const Backends(Buffer const & buffer)
{
	vector<string> v;
	if (buffer.params().getTextClass().isTeXClassAvailable()) {
		v.push_back(bufferFormat(buffer));
		// FIXME: Don't hardcode format names here, but use a flag
		if (v.back() == "latex")
			v.push_back("pdflatex");
	}
	v.push_back("text");
	v.push_back("lyx");
	return v;
}


/// ask the user what to do if a file already exists
int checkOverwrite(FileName const & filename)
{
	if (doesFileExist(filename)) {
		docstring text = bformat(_("The file %1$s already exists.\n\n"
						     "Do you want to overwrite that file?"),
				      makeDisplayPath(filename.absFilename()));
		return Alert::prompt(_("Overwrite file?"),
				     text, 0, 2,
				     _("&Overwrite"), _("Overwrite &all"),
				     _("&Cancel export"));
	}
	return 0;
}


enum CopyStatus {
	SUCCESS,
	FORCE,
	CANCEL
};


/** copy file \p sourceFile to \p destFile. If \p force is false, the user
 *  will be asked before existing files are overwritten.
 *  \return
 *  - SUCCESS if this file got copied
 *  - FORCE   if subsequent calls should not ask for confirmation before
 *            overwriting files anymore.
 *  - CANCEL  if the export should be cancelled
 */
CopyStatus copyFile(string const & format,
		    FileName const & sourceFile, FileName const & destFile,
		    string const & latexFile, bool force)
{
	CopyStatus ret = force ? FORCE : SUCCESS;

	// Only copy files that are in our tmp dir, all other files would
	// overwrite themselves. This check could be changed to
	// boost::filesystem::equivalent(sourceFile, destFile) if export to
	// other directories than the document directory is desired.
	if (!prefixIs(onlyPath(sourceFile.absFilename()), package().temp_dir().absFilename()))
		return ret;

	if (!force) {
		switch(checkOverwrite(destFile)) {
		case 0:
			ret = SUCCESS;
			break;
		case 1:
			ret = FORCE;
			break;
		default:
			return CANCEL;
		}
	}

	Mover const & mover = getMover(format);
	if (!mover.copy(sourceFile, destFile, latexFile))
		Alert::error(_("Couldn't copy file"),
			     bformat(_("Copying %1$s to %2$s failed."),
				     makeDisplayPath(sourceFile.absFilename()),
				     makeDisplayPath(destFile.absFilename())));

	return ret;
}

} //namespace anon


bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir, string & result_file)
{
	string backend_format;
	OutputParams runparams(&buffer->params().encoding());
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = lyxrc.plaintext_linelen;
	vector<string> backends = Backends(*buffer);
	if (find(backends.begin(), backends.end(), format) == backends.end()) {
		// Get shortest path to format
		Graph::EdgePath path;
		for (vector<string>::const_iterator it = backends.begin();
		     it != backends.end(); ++it) {
			Graph::EdgePath p = theConverters().getPath(*it, format);
			if (!p.empty() && (path.empty() || p.size() < path.size())) {
				backend_format = *it;
				path = p;
			}
		}
		if (!path.empty())
			runparams.flavor = theConverters().getFlavor(path);
		else {
			Alert::error(_("Couldn't export file"),
				bformat(_("No information for exporting the format %1$s."),
				   formats.prettyName(format)));
			return false;
		}
	} else {
		backend_format = format;
		// FIXME: Don't hardcode format names here, but use a flag
		if (backend_format == "pdflatex")
			runparams.flavor = OutputParams::PDFLATEX;
	}

	string filename = buffer->getLatexName(false);
	filename = addName(buffer->temppath(), filename);
	filename = changeExtension(filename,
				   formats.extension(backend_format));

	// Plain text backend
	if (backend_format == "text")
		writePlaintextFile(*buffer, FileName(filename), runparams);
	// no backend
	else if (backend_format == "lyx")
		buffer->writeFile(FileName(filename));
	// Docbook backend
	else if (buffer->isDocBook()) {
		runparams.nice = !put_in_tempdir;
		buffer->makeDocBookFile(FileName(filename), runparams);
	}
	// LaTeX backend
	else if (backend_format == format) {
		runparams.nice = true;
		if (!buffer->makeLaTeXFile(FileName(filename), string(), runparams))
			return false;
	} else if (!lyxrc.tex_allows_spaces
		   && contains(buffer->filePath(), ' ')) {
		Alert::error(_("File name error"),
			   _("The directory path to the document cannot contain spaces."));
		return false;
	} else {
		runparams.nice = false;
		if (!buffer->makeLaTeXFile(FileName(filename), buffer->filePath(), runparams))
			return false;
	}

	string const error_type = (format == "program")? "Build" : bufferFormat(*buffer);
	string const ext = formats.extension(format);
	FileName const tmp_result_file(changeExtension(filename, ext));
	bool const success = theConverters().convert(buffer, FileName(filename),
		tmp_result_file, FileName(buffer->fileName()), backend_format, format,
		buffer->errorList(error_type));
	// Emit the signal to show the error list.
	if (format != backend_format)
		buffer->errors(error_type);
	if (!success)
		return false;

	if (put_in_tempdir)
		result_file = tmp_result_file.absFilename();
	else {
		result_file = changeExtension(buffer->fileName(), ext);
		// We need to copy referenced files (e. g. included graphics
		// if format == "dvi") to the result dir.
		vector<ExportedFile> const files =
			runparams.exportdata->externalFiles(format);
		string const dest = onlyPath(result_file);
		CopyStatus status = SUCCESS;
		for (vector<ExportedFile>::const_iterator it = files.begin();
				it != files.end() && status != CANCEL; ++it) {
			string const fmt =
				formats.getFormatFromFile(it->sourceName);
			status = copyFile(fmt, it->sourceName,
					  makeAbsPath(it->exportName, dest),
					  it->exportName, status == FORCE);
		}
		if (status == CANCEL) {
			buffer->message(_("Document export cancelled."));
		} else if (doesFileExist(tmp_result_file)) {
			// Finally copy the main file
			status = copyFile(format, tmp_result_file,
					  FileName(result_file), result_file,
					  status == FORCE);
			buffer->message(bformat(_("Document exported as %1$s "
							       "to file `%2$s'"),
						formats.prettyName(format),
						makeDisplayPath(result_file)));
		} else {
			// This must be a dummy converter like fax (bug 1888)
			buffer->message(bformat(_("Document exported as %1$s"),
						formats.prettyName(format)));
		}
	}

	return true;
}


bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir)
{
	string result_file;
	return Export(buffer, format, put_in_tempdir, result_file);
}


bool Exporter::preview(Buffer * buffer, string const & format)
{
	string result_file;
	if (!Export(buffer, format, true, result_file))
		return false;
	return formats.view(*buffer, FileName(result_file), format);
}


bool Exporter::isExportable(Buffer const & buffer, string const & format)
{
	vector<string> backends = Backends(buffer);
	for (vector<string>::const_iterator it = backends.begin();
	     it != backends.end(); ++it)
		if (theConverters().isReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> const
Exporter::getExportableFormats(Buffer const & buffer, bool only_viewable)
{
	vector<string> backends = Backends(buffer);
	vector<Format const *> result =
		theConverters().getReachable(backends[0], only_viewable, true);
	for (vector<string>::const_iterator it = backends.begin() + 1;
	     it != backends.end(); ++it) {
		vector<Format const *>  r =
			theConverters().getReachable(*it, only_viewable, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


ExportedFile::ExportedFile(FileName const & s, string const & e) :
	sourceName(s), exportName(e) {}


bool operator==(ExportedFile const & f1, ExportedFile const & f2)
{
	return f1.sourceName == f2.sourceName &&
	       f1.exportName == f2.exportName;

}


void ExportData::addExternalFile(string const & format,
				 FileName const & sourceName,
				 string const & exportName)
{
	// Make sure that we have every file only once, otherwise copyFile()
	// would ask several times if it should overwrite a file.
	vector<ExportedFile> & files = externalfiles[format];
	ExportedFile file(sourceName, exportName);
	if (find(files.begin(), files.end(), file) == files.end())
		files.push_back(file);
}


void ExportData::addExternalFile(string const & format,
				 FileName const & sourceName)
{
	addExternalFile(format, sourceName, onlyFilename(sourceName.absFilename()));
}


vector<ExportedFile> const
ExportData::externalFiles(string const & format) const
{
	FileMap::const_iterator cit = externalfiles.find(format);
	if (cit != externalfiles.end())
		return cit->second;
	return vector<ExportedFile>();
}


} // namespace lyx
