/**
 * \file exporter.C
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

#include "exporter.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "converter.h"
#include "format.h"
#include "gettext.h"
#include "lyxrc.h"
#include "mover.h"
#include "output_plaintext.h"
#include "outputparams.h"
#include "frontends/Alert.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/package.h"

#include <boost/filesystem/operations.hpp>

using lyx::support::AddName;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::package;
using lyx::support::prefixIs;

using std::find;
using std::string;
using std::vector;

namespace fs = boost::filesystem;

namespace {

vector<string> const Backends(Buffer const & buffer)
{
	vector<string> v;
	if (buffer.params().getLyXTextClass().isTeXClassAvailable())
		v.push_back(BufferFormat(buffer));
	v.push_back("text");
	return v;
}


/// ask the user what to do if a file already exists
int checkOverwrite(string const & filename)
{
	if (fs::exists(filename)) {
		string text = bformat(_("The file %1$s already exists.\n\n"
					"Do you want to over-write that file?"),
		                      MakeDisplayPath(filename));
		return Alert::prompt(_("Over-write file?"),
		                     text, 0, 2,
		                     _("&Over-write"), _("Over-write &all"),
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
                    string const & sourceFile, string const & destFile,
                    string const & latexFile, bool force)
{
	CopyStatus ret = force ? FORCE : SUCCESS;

	// Only copy files that are in our tmp dir, all other files would
	// overwrite themselves. This check could be changed to
	// boost::filesystem::equivalent(sourceFile, destFile) if export to
	// other directories than the document directory is desired.
	if (!prefixIs(OnlyPath(sourceFile), package().temp_dir()))
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

	Mover const & mover = movers(format);
	if (!mover.copy(sourceFile, destFile, latexFile))
		Alert::error(_("Couldn't copy file"),
		             bformat(_("Copying %1$s to %2$s failed."),
		                     MakeDisplayPath(sourceFile),
		                     MakeDisplayPath(destFile)));

	return ret;
}

} //namespace anon


bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir, string & result_file)
{
	string backend_format;
	OutputParams runparams;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = lyxrc.ascii_linelen;
	vector<string> backends = Backends(*buffer);
	if (find(backends.begin(), backends.end(), format) == backends.end()) {
		for (vector<string>::const_iterator it = backends.begin();
		     it != backends.end(); ++it) {
			Graph::EdgePath p =
				converters.getPath(*it,	format);
			if (!p.empty()) {
				runparams.flavor = converters.getFlavor(p);
				backend_format = *it;
				break;
			}
		}
		if (backend_format.empty()) {
			Alert::error(_("Couldn't export file"),
				bformat(_("No information for exporting the format %1$s."),
				   formats.prettyName(format)));
			return false;
		}
	} else
		backend_format = format;

	string filename = buffer->getLatexName(false);
	filename = AddName(buffer->temppath(), filename);
	filename = ChangeExtension(filename,
				   formats.extension(backend_format));

	// Ascii backend
	if (backend_format == "text")
		writeFileAscii(*buffer, filename, runparams);
	// Linuxdoc backend
	else if (buffer->isLinuxDoc()) {
		runparams.nice = !put_in_tempdir;
		buffer->makeLinuxDocFile(filename, runparams);
	}
	// Docbook backend
	else if (buffer->isDocBook()) {
		runparams.nice = !put_in_tempdir;
		buffer->makeDocBookFile(filename, runparams);
	}
	// LaTeX backend
	else if (backend_format == format) {
		runparams.nice = true;
		buffer->makeLaTeXFile(filename, string(), runparams);
	} else if (!lyxrc.tex_allows_spaces
		   && contains(buffer->filePath(), ' ')) {
		Alert::error(_("File name error"),
			   _("The directory path to the document cannot contain spaces."));
		return false;
	} else {
		runparams.nice = false;
		buffer->makeLaTeXFile(filename, buffer->filePath(), runparams);
	}

	if (!converters.convert(buffer, filename, filename,
				backend_format, format, result_file))
		return false;

	if (!put_in_tempdir) {
		string const tmp_result_file = result_file;
		result_file = ChangeExtension(buffer->fileName(),
		                              formats.extension(format));
		// We need to copy referenced files (e. g. included graphics
		// if format == "dvi") to the result dir.
		vector<ExportedFile> const files =
			runparams.exportdata->externalFiles(format);
		string const dest = OnlyPath(result_file);
		CopyStatus status = SUCCESS;
		for (vector<ExportedFile>::const_iterator it = files.begin();
				it != files.end() && status != CANCEL; ++it) {
			string const fmt =
				formats.getFormatFromFile(it->sourceName);
			status = copyFile(fmt, it->sourceName,
			                  MakeAbsPath(it->exportName, dest),
			                  it->exportName, status == FORCE);
		}
		if (status == CANCEL) {
			buffer->message(_("Document export cancelled."));
		} else {
			// Finally copy the main file
			status = copyFile(format, tmp_result_file,
			                  result_file, result_file,
			                  status == FORCE);
			buffer->message(bformat(_("Document exported as %1$s"
			                          "to file `%2$s'"),
			                        formats.prettyName(format),
			                        MakeDisplayPath(result_file)));
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


bool Exporter::Preview(Buffer * buffer, string const & format)
{
	string result_file;
	if (!Export(buffer, format, true, result_file))
		return false;
	return formats.view(*buffer, result_file, format);
}


bool Exporter::IsExportable(Buffer const & buffer, string const & format)
{
	vector<string> backends = Backends(buffer);
	for (vector<string>::const_iterator it = backends.begin();
	     it != backends.end(); ++it)
		if (converters.isReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> const
Exporter::GetExportableFormats(Buffer const & buffer, bool only_viewable)
{
	vector<string> backends = Backends(buffer);
	vector<Format const *> result =
		converters.getReachable(backends[0], only_viewable, true);
	for (vector<string>::const_iterator it = backends.begin() + 1;
	     it != backends.end(); ++it) {
		vector<Format const *>  r =
			converters.getReachable(*it, only_viewable, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


ExportedFile::ExportedFile(string const & s, string const & e) :
	sourceName(s), exportName(e) {}


bool operator==(ExportedFile const & f1, ExportedFile const & f2)
{
	return f1.sourceName == f2.sourceName &&
	       f1.exportName == f2.exportName;

}


void ExportData::addExternalFile(string const & format,
				 string const & sourceName,
				 string const & exportName)
{
	BOOST_ASSERT(lyx::support::AbsolutePath(sourceName));

	// Make sure that we have every file only once, otherwise copyFile()
	// would ask several times if it should overwrite a file.
	vector<ExportedFile> & files = externalfiles[format];
	ExportedFile file(sourceName, exportName);
	if (find(files.begin(), files.end(), file) == files.end())
		files.push_back(file);
}


void ExportData::addExternalFile(string const & format,
				 string const & sourceName)
{
	addExternalFile(format, sourceName, OnlyFilename(sourceName));
}


vector<ExportedFile> const
ExportData::externalFiles(string const & format) const
{
	FileMap::const_iterator cit = externalfiles.find(format);
	if (cit != externalfiles.end())
		return cit->second;
	return vector<ExportedFile>();
}
