/**
 * \file InsetFootlike.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFootlike.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "MetricsInfo.h"


namespace lyx {


InsetFootlike::InsetFootlike(BufferParams const & bp)
	: InsetCollapsable(bp)
{}


InsetFootlike::InsetFootlike(InsetFootlike const & in)
	: InsetCollapsable(in)
{}


bool InsetFootlike::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Font tmpfont = mi.base.font;
	mi.base.font = mi.base.bv->buffer().params().getFont();
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetFootlike::draw(PainterInfo & pi, int x, int y) const
{
	Font tmpfont = pi.base.font;
	pi.base.font = pi.base.bv->buffer().params().getFont();
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetFootlike::write(Buffer const & buf, std::ostream & os) const
{
	os << to_utf8(name()) << "\n";
	InsetCollapsable::write(buf, os);
}


bool InsetFootlike::insetAllowed(Inset::Code code) const
{
	if (code == Inset::FOOT_CODE || code == Inset::MARGIN_CODE
	    || code == Inset::FLOAT_CODE)
		return false;
	return InsetCollapsable::insetAllowed(code);
}


} // namespace lyx
