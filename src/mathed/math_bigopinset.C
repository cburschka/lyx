#include "math_bigopinset.h"
#include "mathed/math_parser.h"
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
	os << '\\' << sym_->name;
}


void MathBigopInset::writeNormal(ostream & os) const
{
	os << "[bigop " << sym_->name << "] ";
}


void MathBigopInset::metrics(MathStyles st)
{
	size(st);
	
	if (sym_->id > 0 && sym_->id < 256) {
		ssym_ = string();
		ssym_ += sym_->id;
		code_ = LM_TC_BSYM;
	} else {
		ssym_ = sym_->name;
		code_ = LM_TC_TEXTRM;
	}

	mathed_string_dim(code_, size(), ssym_, ascent_, descent_, width_);
}


void MathBigopInset::draw(Painter & pain, int x, int y)
{  
	xo(x);
	yo(y);

	drawStr(pain, code_, size_, x, y, ssym_);
}
