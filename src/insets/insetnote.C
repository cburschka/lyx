/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetnote.h"
#include "gettext.h"
#include "lyxfont.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"

using std::ostream;

void InsetNote::init()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.setColor(LColor::note);
	setLabelFont(font);

	setAutoCollapse(false);
	setBackgroundColor(LColor::note);

	setLabel(_("note"));
	setInsetName("Note");
}


InsetNote::InsetNote()
	: InsetCollapsable()
{
	init();
}


InsetNote::InsetNote(Buffer const * buf, string const & contents, 
		     bool collapsed)
	: InsetCollapsable(collapsed)
{
	init();

	Paragraph * par = inset.paragraph();
	Paragraph::size_type pos = 0;
	buf->insertStringAsLines(par, pos, LyXFont(LyXFont::ALL_INHERIT), 
	                         strip(contents, '\n'));
}


string const InsetNote::editMessage() const 
{
	return _("Opened Note Inset");
}


void InsetNote::write(Buffer const *buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}
