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
#include "converter.h"
#include "format.h"
#include "gettext.h"
#include "latexrunparams.h"
#include "lyxrc.h"

#include "frontends/Alert.h"

#include "support/filetools.h"

using namespace lyx::support;

using std::find;

using std::vector;


namespace {

vector<string> const Backends(Buffer const & buffer)
{
	vector<string> v;
	if (buffer.params.getLyXTextClass().isTeXClassAvailable())
		v.push_back(BufferFormat(buffer));
	v.push_back("text");
	return v;
}

} //namespace anon


bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir, string & result_file)
{
	string backend_format;
	LatexRunParams runparams;
	runparams.flavor = LatexRunParams::LATEX;
	vector<string> backends = Backends(*buffer);
	if (find(backends.begin(), backends.end(), format) == backends.end()) {
		for (vector<string>::const_iterator it = backends.begin();
		     it != backends.end(); ++it) {
			Graph::EdgePath p =
				converters.getPath(*it,	format);
			if (!p.empty()) {
				if (converters.usePdflatex(p))
					runparams.flavor = LatexRunParams::PDFLATEX;
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
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename,
				   formats.extension(backend_format));

	// Ascii backend
	if (backend_format == "text")
		buffer->writeFileAscii(filename, lyxrc.ascii_linelen);
	// Linuxdoc backend
	else if (buffer->isLinuxDoc())
		buffer->makeLinuxDocFile(filename, !put_in_tempdir);
	// Docbook backend
	else if (buffer->isDocBook())
		buffer->makeDocBookFile(filename, !put_in_tempdir);
	// LaTeX backend
	else if (backend_format == format) {
		runparams.nice = true;
		buffer->makeLaTeXFile(filename, string(), runparams);
	} else if (contains(buffer->filePath(), ' ')) {
		Alert::error(_("File name error"),
			   _("The directory path to the document cannot contain spaces."));
		return false;
	} else {
		runparams.nice = false;
		buffer->makeLaTeXFile(filename, buffer->filePath(), runparams);
	}

	string outfile_base = (put_in_tempdir)
		? filename : buffer->getLatexName(false);

	if (!converters.convert(buffer, filename, outfile_base,
				backend_format, format, result_file))
		return false;

	if (!put_in_tempdir)
		buffer->message(_("Document exported as ")
				      + formats.prettyName(format)
				      + _(" to file `")
				      + MakeDisplayPath(result_file) +'\'');
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
