#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_boxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/LOstream.h"


MathBoxInset::MathBoxInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathBoxInset::clone() const
{
	return new MathBoxInset(*this);
}


void MathBoxInset::write(WriteStream & os) const
{
	os << "\\" << name_ << "{" << cell(0) << "}";
}


void MathBoxInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


void MathBoxInset::rebreak()
{
	//lyxerr << "trying to rebreak...\n";
}


void MathBoxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	xcell(0).metrics(mi);
	dim_ = xcell(0).dim();
	metricsMarkers2();
}


void MathBoxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	xcell(0).draw(pi, x, y);
	drawMarkers2(pi, x + 1, y);
}


void MathBoxInset::infoize(std::ostream & os) const
{
	os << "Box: " << name_;
}
