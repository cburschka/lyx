#ifdef __GNUG__
#pragma implementation
#endif

#include "math_binaryopinset.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "support.h"
#include "math_mathmlstream.h"


MathBinaryOpInset::MathBinaryOpInset(char op)
	: MathNestInset(2), op_(op)
{}


MathInset * MathBinaryOpInset::clone() const
{   
	return new MathBinaryOpInset(*this);
}


int MathBinaryOpInset::opwidth() const
{
	return mathed_char_width(LM_TC_CONST, mi_, op_);
}


void MathBinaryOpInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	xcell(0).metrics(mi);
	xcell(1).metrics(mi);
	width_   = xcell(0).width() + xcell(1).width() + opwidth();
	ascent_  = std::max(xcell(0).ascent(),  xcell(1).ascent());
	descent_ = std::max(xcell(0).descent(), xcell(1).descent());
}


void MathBinaryOpInset::draw(Painter & pain, int x, int y) const
{ 
	xcell(0).draw(pain, x, y); 
	drawChar(pain, LM_TC_CONST, mi_, x + xcell(0).width() , y, op_);
	xcell(1).draw(pain, x + width() - xcell(1).width(), y); 
}


void MathBinaryOpInset::write(MathWriteInfo & os) const
{
	os << '{' << cell(0) << op_ << cell(1) << '}';
}


void MathBinaryOpInset::writeNormal(NormalStream & os) const
{
	os << "[binop " << op_ << ' ' << cell(0) << ' ' << cell(1) << ']';
}
