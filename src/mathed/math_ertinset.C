/**
 * \file math_ertinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_ertinset.h"
#include "math_mathmlstream.h"
#include "support/LOstream.h"

using std::auto_ptr;


auto_ptr<InsetBase> MathErtInset::clone() const
{
	return auto_ptr<InsetBase>(new MathErtInset(*this));
}


void MathErtInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "lyxert");
	MathTextInset::metrics(mi, dim_);
	cache_.colinfo_[0].align_ = 'l';
	metricsMarkers();
	dim = dim_;
}


void MathErtInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "lyxert");
	MathTextInset::draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathErtInset::write(WriteStream & os) const
{
	if (os.latex())
		os << cell(0);
	else
		os << "\\lyxert{" << cell(0) << '}';
}


void MathErtInset::infoize(std::ostream & os) const
{
	os << "Box: Ert";
}
