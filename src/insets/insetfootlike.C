/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfootlike.h"
#include "lyxfont.h"
#include "buffer.h"
#include "lyxtext.h"
#include "support/LOstream.h"


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


void InsetFootlike::Write(Buffer const * buf, std::ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}
