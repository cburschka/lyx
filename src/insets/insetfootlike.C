/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2000-2001 The LyX Team.
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
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setAutoCollapse(false);
}


void InsetFootlike::write(Buffer const * buf, std::ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


bool InsetFootlike::insetAllowed(Inset::Code code) const
{
	if ((code == Inset::FOOT_CODE) || (code == Inset::MARGIN_CODE)
	    || (code ==Inset::FLOAT_CODE))
		return false;
	return InsetCollapsable::insetAllowed(code);
}
