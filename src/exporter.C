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
#include <stdio.h>

#include "exporter.h"
#include "converter.h"

#include "buffer.h"
#include "lyx_cb.h"
#include "support/path.h"

using std::vector;
using std::pair;

bool Exporter::Export(Buffer * buffer, string const & format0,
		      bool put_in_tempdir)
{
	string format;
	string using_format = Converter::SplitFormat(format0, format);

	string filename = buffer->fileName();
	string backend_format = BufferExtension(buffer);
	bool only_backend = backend_format == format;

	//string file = buffer->getLatexName(true);
        string file = filename;
	if (!buffer->tmppath.empty())
		file = AddName(buffer->tmppath, file);
	file = ChangeExtension(file, backend_format);

	if (buffer->isLinuxDoc())
		buffer->makeLinuxDocFile(file, only_backend);
	else if (only_backend)
		buffer->makeLaTeXFile(file, string(), true);
	else
		buffer->makeLaTeXFile(file, buffer->filepath, false);

	bool return_value = Converter::convert(buffer, file, format0);
	if (!return_value)
		return false;

	if (!put_in_tempdir) {
		file = ChangeExtension(file, format);
		string outfile = ChangeExtension(filename, format);
		if (file != outfile)
			rename(file.c_str(), outfile.c_str());

		ShowMessage(buffer,
			    _("Document exported as ")
			    + Formats::PrettyName(format)
			    + _(" to file `")
			    + MakeDisplayPath(outfile) +'\'');
	}
	return true;
}


bool Exporter::Preview(Buffer * buffer, string const & format0)
{
	if (!Export(buffer, format0, true))
		return false;

	string format;
	Converter::SplitFormat(format0, format);

	string filename = buffer->fileName();
	if (!buffer->tmppath.empty())
		filename = AddName(buffer->tmppath, filename);
	filename = ChangeExtension(filename, format);
	return Formats::View(filename);
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
