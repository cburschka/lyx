#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_binaryopinset.h"
#include "MathPainterInfo.h"
#include "support/LOstream.h"
#include "math_support.h"
#include "math_mathmlstream.h"


using std::max;


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


void MathBinaryOpInset::metrics(MathMetricsInfo & mi) const
{
	mi_ = mi;
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	width_   = cell(0).width() + cell(1).width() + opwidth();
	ascent_  = max(cell(0).ascent(),  cell(1).ascent());
	descent_ = max(cell(0).descent(), cell(1).descent());
}


void MathBinaryOpInset::draw(MathPainterInfo & pain, int x, int y) const
{
	cell(0).draw(pain, x, y);
	drawChar(pain, LM_TC_CONST, mi_, x + cell(0).width() , y, op_);
	cell(1).draw(pain, x + width() - cell(1).width(), y);
}


void MathBinaryOpInset::write(WriteStream & os) const
{
	os << '{' << cell(0) << op_ << cell(1) << '}';
}


void MathBinaryOpInset::normalize(NormalStream & os) const
{
	os << "[binop " << op_ << ' ' << cell(0) << ' ' << cell(1) << ']';
}
