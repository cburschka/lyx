#include "math_symbolinset.h"
#include "mathed/math_parser.h"
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
	os << "[bigop " << sym_->name << "] ";
}


void MathSymbolInset::metrics(MathStyles st)
{
	size(st);
	
	if (sym_->id > 0 && sym_->id < 256) {
		ssym_ = string();
		ssym_ += sym_->id;
		code_ = (sym_->token == LM_TK_BIGSYM) ? LM_TC_BSYM : LM_TC_SYMB;
	} else {
		ssym_ = sym_->name;
		code_ = LM_TC_TEXTRM;
	}

	mathed_string_dim(code_, size(), ssym_, ascent_, descent_, width_);
}


void MathSymbolInset::draw(Painter & pain, int x, int y)
{  
	xo(x);
	yo(y);

	drawStr(pain, code_, size_, x, y, ssym_);
}
