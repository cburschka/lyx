#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sizeinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


MathSizeInset::MathSizeInset(latexkeys const * l)
	: MathNestInset(1), key_(l)
{}


MathInset * MathSizeInset::clone() const
{
	return new MathSizeInset(*this);
}


void MathSizeInset::draw(Painter & pain, int x, int y) const
{
	xcell(0).draw(pain, x + 1, y);
	mathed_draw_framebox(pain, x, y, this);
}


void MathSizeInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	m.style = MathStyles(key_->id);
	xcell(0).metrics(m);
	ascent_   = xcell(0).ascent_ + 1;
	descent_  = xcell(0).descent_ + 1;
	width_    = xcell(0).width_ + 2;
}


void MathSizeInset::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void MathSizeInset::normalize(NormalStream & os) const
{
	os << "[" << key_->name << ' ' << cell(0) << ']';
}
