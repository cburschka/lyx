#include "math_noglyphinset.h"
#include "math_parser.h"
#include "support.h"
#include "support/LOstream.h"


using std::ostream;

MathNoglyphInset::MathNoglyphInset(const latexkeys * l)
	: sym_(l)
{}


MathInset * MathNoglyphInset::clone() const
{
	return new MathNoglyphInset(*this);
}


void MathNoglyphInset::write(ostream & os, bool /* fragile */) const
{
	os << '\\' << sym_->name << ' ';
}


void MathNoglyphInset::writeNormal(ostream & os) const
{
	os << "[bigop " << sym_->name << "] ";
}


void MathNoglyphInset::metrics(MathStyles st) const
{
	size(st);
	mathed_string_dim(LM_TC_TEX, st, sym_->name, ascent_, descent_, width_);
}


void MathNoglyphInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);
	drawStr(pain, LM_TC_TEX, size_, x, y, sym_->name);
}
