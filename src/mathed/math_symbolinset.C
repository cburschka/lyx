#include "math_symbolinset.h"
#include "math_parser.h"
#include "support.h"
#include "support/LOstream.h"


using std::ostream;

MathSymbolInset::MathSymbolInset(const latexkeys * l)
	: sym_(l)
{}


MathInset * MathSymbolInset::clone() const
{
	return new MathSymbolInset(*this);
}


void MathSymbolInset::write(ostream & os, bool /* fragile */) const
{
	os << '\\' << sym_->name << ' ';
}


void MathSymbolInset::writeNormal(ostream & os) const
{
	os << "[" << sym_->name << "] ";
}


void MathSymbolInset::metrics(MathStyles st) const
{
	size(st);
	mathed_char_dim(LM_TC_SYMB, size_, sym_->id, ascent_, descent_, width_);
}


void MathSymbolInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);

	drawChar(pain, LM_TC_SYMB, size_, x, y, sym_->id);
}


bool MathSymbolInset::isRelOp() const
{
	return sym_->id == LM_leq || sym_->id == LM_geq;
}
