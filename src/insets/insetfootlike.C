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

#include "insetfootlike.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "support/LOstream.h"

using std::ostream;
using std::endl;

InsetFootlike::InsetFootlike()
	: InsetCollapsable()
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::footnote);
	setLabelFont(font);
	setAutoCollapse(false);
}


void InsetFootlike::Write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}


#if 0
LyXFont InsetFootlike::GetDrawFont(BufferView * bv,
				   LyXParagraph * p, int pos) const
{
	LyXFont fn = getLyXText(bv)->GetFont(bv->buffer(), p, pos);
	fn.decSize().decSize();
	return fn;
}
#endif
