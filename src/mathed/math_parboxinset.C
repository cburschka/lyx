/**
 * \file math_parboxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_parboxinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxlength.h"
#include "debug.h"

using std::auto_ptr;
using std::endl;


MathParboxInset::MathParboxInset()
	: lyx_width_(0), tex_width_("0mm"), position_('c')
{
	lyxerr << "constructing MathParboxInset" << endl;
}


auto_ptr<InsetBase> MathParboxInset::clone() const
{
	return auto_ptr<InsetBase>(new MathParboxInset(*this));
}


void MathParboxInset::setPosition(string const & p)
{
	position_ = p.size() > 0 ? p[0] : 'c';
}


void MathParboxInset::setWidth(string const & w)
{
	tex_width_ = w;
	lyx_width_ = LyXLength(w).inBP();
	lyxerr << "setting " << w << " to " << lyx_width_ << " pixel" << endl;
}


void MathParboxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy1(mi.base, "textnormal");
	WidthChanger dummy2(mi.base, lyx_width_);
	MathTextInset::metrics(mi, dim_);
	metricsMarkers();
	dim = dim_;
}


void MathParboxInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	MathTextInset::draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathParboxInset::write(WriteStream & os) const
{
	os << "\\parbox";
	if (position_ != 'c')
		os << '[' << position_ << ']';
	os << '{' << tex_width_ << "}{" << cell(0) << '}';
}


void MathParboxInset::infoize(std::ostream & os) const
{
	os << "Box: Parbox " << tex_width_;
}
