#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sizeinset.h"
#include "math_parser.h"
#include "support/LOstream.h"


MathSizeInset::MathSizeInset(latexkeys const * l)
	: MathNestInset(1), key_(l)
{}


MathInset * MathSizeInset::clone() const
{
	return new MathSizeInset(*this);
}


void MathSizeInset::draw(Painter & pain, int x, int y) const
{
	xcell(0).draw(pain, x, y);
}


void MathSizeInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	m.style = MathStyles(key_->id);
	xcell(0).metrics(m);
	ascent_   = xcell(0).ascent_;
	descent_  = xcell(0).descent_;
	width_    = xcell(0).width_;
}


void MathSizeInset::write(MathWriteInfo & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void MathSizeInset::writeNormal(std::ostream & os) const
{
	os << "[" << key_->name << " ";
	cell(0).writeNormal(os);
	os << "]";
}
