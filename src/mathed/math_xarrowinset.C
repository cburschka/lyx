#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xarrowinset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathXArrowInset::MathXArrowInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathXArrowInset::clone() const
{   
	return new MathXArrowInset(*this);
}


void MathXArrowInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	smallerStyleScript(mi);
	xcell(0).metrics(mi);
	width_   = xcell(0).width() + 10;
	ascent_  = xcell(0).height() + 10;
	descent_ = 0;
}


void MathXArrowInset::draw(Painter & pain, int x, int y) const
{ 
	xcell(0).draw(pain, x + 5, y - 10);
	mathed_draw_deco(pain, x + 1, y - 7, width_ - 2, 5, name_);
}


void MathXArrowInset::write(WriteStream & os) const
{
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void MathXArrowInset::normalize(NormalStream & os) const
{
	os << "[xarrow " << name_ << ' ' <<  cell(0) << ']';
}
