/**
 * \file insetfootlike.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetfootlike.h"

#include "buffer.h"
#include "LColor.h"
#include "metricsinfo.h"
#include "paragraph.h"

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


InsetFootlike::InsetFootlike(InsetFootlike const & in)
	: InsetCollapsable(in)
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
}


void InsetFootlike::write(Buffer const & buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


bool InsetFootlike::insetAllowed(InsetBase::Code code) const
{
	if (code == InsetBase::FOOT_CODE || code == InsetBase::MARGIN_CODE
	    || code == InsetBase::FLOAT_CODE)
		return false;
	return InsetCollapsable::insetAllowed(code);
}
