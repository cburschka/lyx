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
#include "Font.h"
#include "MetricsInfo.h"

#include <iostream>

using namespace std;

namespace lyx {


InsetFootlike::InsetFootlike(Buffer * buf)
	: InsetCollapsable(buf)
{}


void InsetFootlike::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontInfo tmpfont = mi.base.font;
	mi.base.font = mi.base.bv->buffer().params().getFont().fontInfo();
	InsetCollapsable::metrics(mi, dim);
	mi.base.font = tmpfont;
}


void InsetFootlike::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo tmpfont = pi.base.font;
	pi.base.font = pi.base.bv->buffer().params().getFont().fontInfo();
	InsetCollapsable::draw(pi, x, y);
	pi.base.font = tmpfont;
}


void InsetFootlike::write(ostream & os) const
{
	os << to_utf8(layoutName()) << "\n";
	InsetCollapsable::write(os);
}


bool InsetFootlike::insetAllowed(InsetCode code) const
{
	if (code == FOOT_CODE || code == MARGIN_CODE || code == FLOAT_CODE
		|| code == LISTINGS_CODE)
		return false;
	return InsetCollapsable::insetAllowed(code);
}


} // namespace lyx
