#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_boxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathBoxInset::MathBoxInset(string const & name)
	: MathGridInset(1, 1), name_(name)
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
	MathGridInset::metrics(mi);
}


void MathBoxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	MathGridInset::draw(pi, x, y);
	mathed_draw_framebox(pi, x, y, this);
}


void MathBoxInset::infoize(std::ostream & os) const
{
	os << "Box: " << name_;
}
