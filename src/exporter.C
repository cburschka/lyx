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

#include "exporter.h"
#include "buffer.h"
#include "lyx_cb.h" //ShowMessage()
#include "support/filetools.h"
#include "lyxrc.h"
#include "converter.h"

using std::vector;

bool Exporter::Export(Buffer * buffer, string const & format0,
		      bool put_in_tempdir, string * view_file)
{
	string format;
	string using_format = Converter::SplitFormat(format0, format);

	string backend_format = (format == "txt") 
		? format : BufferExtension(buffer);
	bool only_backend = backend_format == format;

	string filename = buffer->getLatexName(false);
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, backend_format);

	// Ascii backend
	if (backend_format == "txt")
		buffer->writeFileAscii(filename, lyxrc.ascii_linelen);
	// Linuxdoc backend
	else if (buffer->isLinuxDoc())
		buffer->makeLinuxDocFile(filename, true);
	// Docbook backend
	else if (buffer->isDocBook())
		buffer->makeDocBookFile(filename, true);
	// LaTeX backend
	else if (only_backend)
		buffer->makeLaTeXFile(filename, string(), true);
	else
		buffer->makeLaTeXFile(filename, buffer->filepath, false);

	string outfile = (put_in_tempdir)
		? ChangeExtension(filename, format)
		: ChangeExtension(buffer->getLatexName(false), format);

	if (!Converter::Convert(buffer, filename, outfile, using_format, 
				view_file))
		return false;

	if (!put_in_tempdir)
		ShowMessage(buffer,
			    _("Document exported as ")
			    + Formats::PrettyName(format)
			    + _(" to file `")
			    + MakeDisplayPath(outfile) +'\'');
	return true;
}


bool Exporter::Preview(Buffer * buffer, string const & format0)
{
	string view_file;
	if (!Export(buffer, format0, true, &view_file))
		return false;

	return Formats::View(buffer, view_file);
}


bool Exporter::IsExportable(Buffer const * buffer, string const & format)
{
	return format == "txt" ||
		Converter::IsReachable(BufferExtension(buffer), format);
}


vector<FormatPair> const
Exporter::GetExportableFormats(Buffer const * buffer)
{
	vector<FormatPair> result = 
		Converter::GetReachable(BufferExtension(buffer), false);
	Format * format = Formats::GetFormat("txt");
	if (format)
		result.push_back(FormatPair(format , 0, ""));
	return result;
}


vector<FormatPair> const
Exporter::GetViewableFormats(Buffer const * buffer)
{
	vector<FormatPair> result = 
		Converter::GetReachable(BufferExtension(buffer), true);
	Format * format = Formats::GetFormat("txt");
	if (format && !format->viewer.empty())
		result.push_back(FormatPair(format , 0, ""));
	return result;
}


string const Exporter::BufferExtension(Buffer const * buffer)
{
	if (buffer->isLinuxDoc())
		return "sgml";
	else if (buffer->isDocBook())
		return "docbook";
	else if (buffer->isLiterate())
		return lyxrc.literate_extension;
	else
		return "tex";
}
