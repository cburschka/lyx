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
		      bool put_in_tempdir, string & result_file)
{
	string format;
	string using_format = Converter::SplitFormat(format0, format);

	string backend_format = (format == "text") 
		? format : BufferFormat(buffer);
	bool only_backend = backend_format == format;

	string filename = buffer->getLatexName(false);
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, 
				   Formats::Extension(backend_format));

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
	else if (only_backend)
		buffer->makeLaTeXFile(filename, string(), true);
	else
		buffer->makeLaTeXFile(filename, buffer->filepath, false);

	string outfile_base = (put_in_tempdir)
		? filename : buffer->getLatexName(false);

	if (!Converter::Convert(buffer, filename, outfile_base,
				backend_format, format, using_format,
				result_file))
		return false;

	if (!put_in_tempdir)
		ShowMessage(buffer,
			    _("Document exported as ")
			    + Formats::PrettyName(format)
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

bool Exporter::Preview(Buffer * buffer, string const & format0)
{
	string result_file;
	if (!Export(buffer, format0, true, result_file))
		return false;
	string format;
	Converter::SplitFormat(format0, format);
	return Formats::View(buffer, result_file, format);
}


bool Exporter::IsExportable(Buffer const * buffer, string const & format)
{
	return format == "text" ||
		Converter::IsReachable(BufferFormat(buffer), format);
}


vector<FormatPair> const
Exporter::GetExportableFormats(Buffer const * buffer)
{
	vector<FormatPair> result = 
		Converter::GetReachable(BufferFormat(buffer), "lyx", false);
	Format * format = Formats::GetFormat("text");
	if (format)
		result.push_back(FormatPair(format , 0, ""));
	return result;
}


vector<FormatPair> const
Exporter::GetViewableFormats(Buffer const * buffer)
{
	vector<FormatPair> result = 
		Converter::GetReachable(BufferFormat(buffer), "lyx", true);
	Format * format = Formats::GetFormat("text");
	if (format && !format->viewer.empty())
		result.push_back(FormatPair(format , 0, ""));
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
