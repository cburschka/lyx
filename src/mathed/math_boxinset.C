/**
 * \file math_boxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_boxinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/std_ostream.h"

using std::auto_ptr;


MathBoxInset::MathBoxInset(string const & name)
	: MathNestInset(1), name_(name)
{}


auto_ptr<InsetBase> MathBoxInset::clone() const
{
	return auto_ptr<InsetBase>(new MathBoxInset(*this));
}


void MathBoxInset::write(WriteStream & os) const
{
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void MathBoxInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


void MathBoxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim_);
	metricsMarkers();
	dim = dim_;
}


void MathBoxInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x, y);
	drawMarkers(pi, x, y);
}


void MathBoxInset::infoize(std::ostream & os) const
{
	os << "Box: " << name_;
}
