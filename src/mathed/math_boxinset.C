#include <config.h>

#include "math_boxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/LOstream.h"


MathBoxInset::MathBoxInset(string const & name)
	: MathNestInset(1), name_(name)
{}


InsetBase * MathBoxInset::clone() const
{
	return new MathBoxInset(*this);
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
