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
#include "converter.h"
#include "buffer.h"
#include "lyx_cb.h" //ShowMessage()
#include "support/filetools.h"

using std::vector;
using std::pair;

bool Exporter::Export(Buffer * buffer, string const & format0,
		      bool put_in_tempdir)
{
	string format;
	string using_format = Converter::SplitFormat(format0, format);

	string backend_format = BufferExtension(buffer);
	bool only_backend = backend_format == format;

	string filename = buffer->getLatexName(false);
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, backend_format);

	if (buffer->isLinuxDoc())
		buffer->makeLinuxDocFile(filename, only_backend);
	else if (only_backend)
		buffer->makeLaTeXFile(filename, string(), true);
	else
		buffer->makeLaTeXFile(filename, buffer->filepath, false);

	string outfile = (put_in_tempdir)
		? ChangeExtension(filename, format)
		: ChangeExtension(buffer->getLatexName(false), format);

	if (!Converter::Convert(buffer, filename, outfile, using_format))
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
	if (!Export(buffer, format0, true))
		return false;

	string format;
	Converter::SplitFormat(format0, format);

	string filename = buffer->getLatexName(false);
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, format);
	return Formats::View(buffer, filename);
}


vector<pair<string, string> > const
Exporter::GetExportableFormats(Buffer const * buffer)
{
	return Converter::GetReachable(BufferExtension(buffer), false);
}


vector<pair<string, string> > const
Exporter::GetViewableFormats(Buffer const * buffer)
{
	return Converter::GetReachable(BufferExtension(buffer), true);
}


string const Exporter::BufferExtension(Buffer const * buffer)
{
	if (buffer->isLinuxDoc())
		return "sgml";
	else
		return "tex";
}
