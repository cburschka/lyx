/**
 * \file insetfootlike.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
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


using std::ostream;


InsetFootlike::InsetFootlike(BufferParams const & bp)
	: InsetCollapsable(bp)
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
}


InsetFootlike::InsetFootlike(InsetFootlike const & in, bool same_id)
	: InsetCollapsable(in, same_id)
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
}


void InsetFootlike::write(Buffer const * buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


bool InsetFootlike::insetAllowed(Inset::Code code) const
{
	if ((code == Inset::FOOT_CODE) || (code == Inset::MARGIN_CODE)
	    || (code == Inset::FLOAT_CODE))
		return false;
	return InsetCollapsable::insetAllowed(code);
}
