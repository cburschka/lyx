/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlabel.h"
#include "support/LOstream.h"
#include "lyx_gui_misc.h"     //askForText
#include "support/lstrings.h" //frontStrip, strip
#include "lyxtext.h"
#include "buffer.h"
#include "gettext.h"
#include "BufferView.h"
#include "support/lstrings.h"

using std::ostream;
using std::vector;
using std::pair;

/* Label. Used to insert a label automatically */


InsetLabel::InsetLabel(InsetCommandParams const & p)
	: InsetCommand(p)
{}


vector<string> const InsetLabel::getLabelList() const
{
	return vector<string>(1, getContents());
}


void InsetLabel::edit(BufferView * bv, int, int, unsigned int)
{
	if (bv->buffer()->isReadonly()) {
		WarnReadonly(bv->buffer()->fileName());
		return;
	}

	pair<bool, string> result = askForText(_("Enter label:"), getContents());
	if (result.first) {
		string new_contents = frontStrip(strip(result.second));
		if (!new_contents.empty() &&
		    getContents() != new_contents) {
			bv->buffer()->markDirty();
			bool flag = bv->ChangeRefsIfUnique(getContents(),
							   new_contents);
			setContents(new_contents);
			bv->text->redoParagraph(bv);
			if (flag) {
				bv->redraw();
				bv->fitCursor(getLyXText(bv));
			} else
				bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
	}
}


int InsetLabel::latex(Buffer const *, ostream & os,
		      bool /*fragile*/, bool /*fs*/) const
{
	os << escape(getCommand());
	return 0;
}

int InsetLabel::ascii(Buffer const *, ostream & os, int) const
{
	os << "<" << getContents()  << ">";
	return 0;
}


int InsetLabel::linuxdoc(Buffer const *, ostream & os) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::docBook(Buffer const *, ostream & os) const
{
	os << "<anchor id=\"" << getContents() << "\" ></anchor>";
	return 0;
}
