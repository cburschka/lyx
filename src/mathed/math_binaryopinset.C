#include <config.h>


#include "math_binaryopinset.h"
#include "PainterInfo.h"
#include "support/LOstream.h"
#include "math_support.h"
#include "math_mathmlstream.h"


using std::max;


MathBinaryOpInset::MathBinaryOpInset(char op)
	: MathNestInset(2), op_(op)
{}


InsetBase * MathBinaryOpInset::clone() const
{
	return new MathBinaryOpInset(*this);
}


int MathBinaryOpInset::opwidth() const
{
	return mathed_char_width(LM_TC_CONST, mi_, op_);
}


#warning Andre, have a look here. (Lgb)
#if 0
// That this is not declared in class MathBinaryOpInset makes
// Doxygen give warnings. (Lgb)
void MathBinaryOpInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mi_ = mi;
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	width_   = cell(0).width() + cell(1).width() + opwidth();
	ascent_  = max(cell(0).ascent(),  cell(1).ascent());
	descent_ = max(cell(0).descent(), cell(1).descent());
}
#endif

void MathBinaryOpInset::draw(PainterInfo & pain, int x, int y) const
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
