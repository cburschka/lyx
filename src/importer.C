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

#include "importer.h"
#include "converter.h"
#include "LyXView.h"
#include "lyxfunc.h"

#include "bufferlist.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h" //WriteAlert
#include "gettext.h"
#include "BufferView.h"

using std::vector;
using std::find;

extern BufferList bufferlist;
extern void InsertAsciiFile(BufferView *, string const &, bool);


bool Importer::Import(LyXView * lv, string const & filename, 
		      string const & format)
{
	string const displaypath = MakeDisplayPath(filename);
	ostringstream s1;
	s1 << _("Importing") << ' ' << displaypath << "...";
	lv->message(s1.str().c_str());

	string const lyxfile = ChangeExtension(filename, ".lyx");

	string loader_format;
	vector<string> loaders = Loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		for (vector<string>::const_iterator it = loaders.begin();
		     it != loaders.end(); ++it) {
			if (converters.isReachable(format, *it)) {
				if (!converters.convert(0, filename, filename,
							format, *it))
					return false;
				loader_format = *it;
				break;
			}
		}
		if (loader_format.empty()) {
			WriteAlert(_("Cannot import file"),
				   _("No information for importing from ")
				   + formats.prettyName(format));
			return false;
		}
	} else
		loader_format = format;


	if (loader_format == "lyx") {
		Buffer * buffer = bufferlist.loadLyXFile(lyxfile);
		if (buffer)
			lv->view()->buffer(buffer);
	} else {
		lv->view()->buffer(bufferlist.newFile(lyxfile, string(), true));
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename
			: ChangeExtension(filename,
					  formats.extension(loader_format));
		InsertAsciiFile(lv->view(), filename2, as_paragraphs);
		lv->getLyXFunc()->dispatch(LFUN_MARK_OFF);
	}

	// we are done
	lv->message(_("imported."));
	return true;
}


#if 0
bool Importer::IsImportable(string const & format)
{
	vector<string> loaders = Loaders();
	for (vector<string>::const_iterator it = loaders.begin();
	     it != loaders.end(); ++it)
		if (converters.IsReachable(format, *it))
			return true;
	return false;
}
#endif


vector<Format const *> const Importer::GetImportableFormats()
{
	vector<string> loaders = Loaders();
	vector<Format const *> result = 
		converters.getReachableTo(loaders[0], true);
	for (vector<string>::const_iterator it = loaders.begin() + 1;
	     it != loaders.end(); ++it) {
		vector<Format const *> r =
			converters.getReachableTo(*it, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


vector<string> const Importer::Loaders()
{
	vector<string> v;
	v.push_back("lyx");
	v.push_back("text");
	v.push_back("textparagraph");
	return v;
}
