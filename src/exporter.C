/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
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
#include "frontends/Alert.h"
#include "gettext.h"
#include "BufferView.h"

using std::vector;
using std::find;

bool Exporter::Export(Buffer * buffer, string const & format,
		      bool put_in_tempdir, string & result_file)
{
	// There are so many different places that this function can be called
	// from that the removal of auto insets is best done here.  This ensures
	// we always have a clean buffer for inserting errors found during export.
	BufferView * bv = buffer->getUser();
	if (bv) {
		// Remove all error insets
		if (bv->removeAutoInsets()) {
			bv->redraw();
			bv->fitCursor();
		}
	}

	string backend_format;
	vector<string> backends = Backends(buffer);
	if (find(backends.begin(), backends.end(), format) == backends.end()) {
		for (vector<string>::const_iterator it = backends.begin();
		     it != backends.end(); ++it) {
			Converters::EdgePath p =
				converters.getPath(*it,	format);
			if (!p.empty()) {
				lyxrc.pdf_mode = converters.usePdflatex(p);
				backend_format = *it;
				break;
			}
		}
		if (backend_format.empty()) {
			Alert::alert(_("Cannot export file"),
				   _("No information for exporting to ")
				   + formats.prettyName(format));
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
	else if (backend_format == format)
		buffer->makeLaTeXFile(filename, string(), true);
	else if (contains(buffer->filePath(), ' ')) {
		Alert::alert(_("Cannot run latex."),
			   _("The path to the lyx file cannot contain spaces."));
		return false;
	} else
		buffer->makeLaTeXFile(filename, buffer->filePath(), false);

	string outfile_base = (put_in_tempdir)
		? filename : buffer->getLatexName(false);

	if (!converters.convert(buffer, filename, outfile_base,
				backend_format, format, result_file))
		return false;

	if (!put_in_tempdir)
		ShowMessage(buffer,
			    _("Document exported as ")
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
	return formats.view(buffer, result_file, format);
}


bool Exporter::IsExportable(Buffer const * buffer, string const & format)
{
	vector<string> backends = Backends(buffer);
	for (vector<string>::const_iterator it = backends.begin();
	     it != backends.end(); ++it)
		if (converters.isReachable(*it, format))
			return true;
	return false;
}


vector<Format const *> const
Exporter::GetExportableFormats(Buffer const * buffer, bool only_viewable)
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
