/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
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

using std::ostream;
using std::vector;
using std::pair;

/* Label. Used to insert a label automatically */


InsetLabel::InsetLabel(string const & cmd)
{
	scanCommand(cmd);
}


Inset * InsetLabel::Clone() const
{
	return new InsetLabel(getCommand());
}


vector<string> InsetLabel::getLabelList() const
{
	return vector<string>(1,getContents());
}


void InsetLabel::Edit(BufferView * bv, int, int, unsigned int)
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
			bool flag = bv->ChangeRefs(getContents(),new_contents);
			setContents( new_contents );
			bv->text->RedoParagraph();
			if (flag) {
				bv->redraw();
				bv->fitCursor();
				//bv->updateScrollbar();
			} else
				bv->update(1);
		}
	}
}

int InsetLabel::Latex(ostream & os,
		      bool /*fragile*/, bool /*fs*/) const
{
	os << escape(getCommand());
	return 0;
}

int InsetLabel::Ascii(ostream & os) const
{
	os << "<" << getContents()  << ">";
	return 0;
}


int InsetLabel::Linuxdoc(ostream & os) const
{
	os << "<label id=\"" << getContents() << "\" >";
	return 0;
}


int InsetLabel::DocBook(ostream & os) const
{
	os << "<anchor id=\"" << getContents() << "\" >";
	return 0;
}


// This function escapes 8-bit characters and other problematic characters
// It's exactly the same code as in insetref.C.
string InsetLabel::escape(string const & lab) const {
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string enc;
	for (string::size_type i= 0; i < lab.length(); ++i) {
		unsigned char c = lab[i];
		if (c >= 128 || c == '=' || c == '%') {
			enc += '=';
			enc += hexdigit[c >> 4];
			enc += hexdigit[c & 15];
		} else {
			enc += c;
		}
	}
	return enc;
}
