#include "math_bigopinset.h"
#include "mathed/math_parser.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

MathBigopInset::MathBigopInset(const latexkeys * l)
	: sym_(l)
{}


MathInset * MathBigopInset::clone() const
{
	return new MathBigopInset(*this);
}


void MathBigopInset::write(ostream & os, bool /* fragile */) const
{
	os << '\\' << sym_->name << ' ';
}


void MathBigopInset::writeNormal(ostream & os) const
{
	os << "[bigop " << sym_->name << "] ";
}


void MathBigopInset::metrics(MathStyles st) const
{
	size_ = st;
	mathed_char_dim(LM_TC_BSYM, st, sym_->id, ascent_, descent_, width_);
}


void MathBigopInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);
	drawChar(pain, LM_TC_BSYM, size_, x, y, sym_->id);
}
