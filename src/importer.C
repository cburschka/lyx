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

#include "importer.h"
#include "converter.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "minibuffer.h"
#include "bufferlist.h"
#include "support/filetools.h"

extern BufferList bufferlist;
extern void InsertAsciiFile(BufferView *, string const &, bool);


void Importer::Import(LyXView * lv, string const & filename, 
		      string const & format)
{
	string displaypath = MakeDisplayPath(filename);
	lv->getMiniBuffer()->Set(_("Importing"), displaypath, "...");

	string lyxfile = ChangeExtension(filename, ".lyx");

	bool result = true;
	if (format == "text" || format == "textparagraph") {
		lv->view()->buffer(bufferlist.newFile(lyxfile, string(), true));
		bool as_paragraphs = format == "textparagraph";
		InsertAsciiFile(lv->view(), filename, as_paragraphs);
		lv->getLyXFunc()->Dispatch(LFUN_MARK_OFF);
	} else {
		result = Converter::Convert(0, filename, filename, format, "lyx");
		if (result) {
			Buffer * buffer = bufferlist.loadLyXFile(lyxfile);
			if (buffer)
				lv->view()->buffer(buffer);
			else
				result = false;
		}
	}

	// we are done
	if (result)
		lv->getMiniBuffer()->Set(displaypath, _("imported."));
	else
		lv->getMiniBuffer()->Set(displaypath, _(": import failed."));

}


bool Importer::IsImportable(string const & format)
{
	if (format == "text" || format == "textparagraph")
		return true;
	else
		return Converter::IsReachable(format, "lyx");
}
