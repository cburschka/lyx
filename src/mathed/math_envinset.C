#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_envinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathEnvInset::MathEnvInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathEnvInset::clone() const
{
	return new MathEnvInset(*this);
}


void MathEnvInset::metrics(MathMetricsInfo & mi) const
{
	xcell(0).metrics(mi);
	ascent_  = xcell(0).ascent() + 1;
	descent_ = xcell(0).descent() + 1;
	width_   = xcell(0).width() + 2;
}


void MathEnvInset::draw(MathPainterInfo & pi, int x, int y) const
{
	xcell(0).draw(pi, x + 1, y);
	drawMarkers2(pi, x, y);
}


void MathEnvInset::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}' << cell(0) << "\\end{" << name_ << '}';
}


void MathEnvInset::normalize(NormalStream & os) const
{
	os << "[env " << name_ << " " << cell(0) << ']';
}


void MathEnvInset::infoize(std::ostream & os) const
{
	os << "Env: " << name_;
}
