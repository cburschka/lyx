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
#include "metricsinfo.h"

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


void InsetFootlike::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	if (isOpen())
		dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetFootlike::write(Buffer const & buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


bool InsetFootlike::insetAllowed(InsetOld::Code code) const
{
	if (code == InsetOld::FOOT_CODE || code == InsetOld::MARGIN_CODE
	    || code == InsetOld::FLOAT_CODE)
		return false;
	return InsetCollapsable::insetAllowed(code);
}
