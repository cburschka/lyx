/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "exporter.h"
#include "buffer.h"
#include "lyx_cb.h" //ShowMessage()
#include "support/filetools.h"
#include "lyxrc.h"
#include "converter.h"
#include "lyx_gui_misc.h" //WriteAlert
#include "gettext.h"

using std::vector;
using std::find;

bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir, string & result_file)
{
	string backend_format;
	vector<string> backends = Backends(buffer);
	if (find(backends.begin(), backends.end(), format) == backends.end()) {
		for (vector<string>::const_iterator it = backends.begin();
		     it != backends.end(); ++it) {
			Converters::EdgePath p =
				converters.GetPath(*it,	format);
			if (!p.empty()) {
				lyxrc.pdf_mode = converters.UsePdflatex(p);
				backend_format = *it;
				break;
			}
		}
		if (backend_format.empty()) {
			WriteAlert(_("Can not export file"),
				   _("No information for exporting to ")
				   + formats.PrettyName(format));
			return false;
		}
	} else
		backend_format = format;

	string filename = buffer->getLatexName(false);
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, 
				   formats.Extension(backend_format));

	// Ascii backend
	if (backend_format == "text")
		buffer->writeFileAscii(filename, lyxrc.ascii_linelen);
	// Linuxdoc backend
	else if (buffer->isLinuxDoc())
		buffer->makeLinuxDocFile(filename, true);
	// Docbook backend
	else if (buffer->isDocBook())
		buffer->makeDocBookFile(filename, true);
	// LaTeX backend
	else if (backend_format == format)
		buffer->makeLaTeXFile(filename, string(), true);
	else
		buffer->makeLaTeXFile(filename, buffer->filepath, false);

	string outfile_base = (put_in_tempdir)
		? filename : buffer->getLatexName(false);

	if (!converters.Convert(buffer, filename, outfile_base,
				backend_format, format, result_file))
		return false;

	if (!put_in_tempdir)
		ShowMessage(buffer,
			    _("Document exported as ")
			    + formats.PrettyName(format)
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
	return formats.View(buffer, result_file, format);
}


bool Exporter::IsExportable(Buffer const * buffer, string const & format)
{
	vector<string> backends = Backends(buffer);
	for (vector<string>::const_iterator it = backends.begin();
	     it != backends.end(); ++it)
		if (converters.IsReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> const
Exporter::GetExportableFormats(Buffer const * buffer, bool only_viewable)
{
	vector<string> backends = Backends(buffer);
	vector<Format const *> result = 
		converters.GetReachable(backends[0], only_viewable, true);
	for (vector<string>::const_iterator it = backends.begin() + 1;
	     it != backends.end(); ++it) {
		vector<Format const *>  r =
			converters.GetReachable(*it, only_viewable, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


string const Exporter::BufferFormat(Buffer const * buffer)
{
	if (buffer->isLinuxDoc())
		return "linuxdoc";
	else if (buffer->isDocBook())
		return "docbook";
	else if (buffer->isLiterate())
		return "literate";
	else
		return "latex";
}

vector<string> const Exporter::Backends(Buffer const * buffer)
{
	vector<string> v;
	v.push_back(BufferFormat(buffer));
	v.push_back("text");
	return v;
}
