#include "math_symbolinset.h"
#include "math_parser.h"
#include "support.h"
#include "support/LOstream.h"


using std::ostream;

MathSymbolInset::MathSymbolInset(const latexkeys * l)
	: sym_(l), h_(0) {}


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


MathTextCodes MathSymbolInset::code() const
{
	switch(sym_->token) {
	case LM_TK_CMR:
		return LM_TC_CMR;
	case LM_TK_CMSY:
		return LM_TC_CMSY;
	case LM_TK_CMM:
		return LM_TC_CMM;
	case LM_TK_CMEX:
		return LM_TC_CMEX;
	case LM_TK_MSA:
		return LM_TC_MSA;
	case LM_TK_MSB:
		return LM_TC_MSB;
	default:
		return LM_TC_SYMB;
	}
}


MathTextCodes MathSymbolInset::code2() const
{
	if (sym_->token == LM_TK_CMEX)
		return LM_TC_BOLDSYMB;
	else
		return LM_TC_SYMB;
}


void MathSymbolInset::metrics(MathStyles st) const
{
	size_ = st;
	MathTextCodes Code = code();
	if (sym_->latex_font_id > 0 && math_font_available(Code)) {
		mathed_char_dim(Code, size_, sym_->latex_font_id,
				ascent_, descent_, width_);
		if (Code == LM_TC_CMEX) {
			h_ = 4*descent_/5;
			ascent_  += h_;
			descent_ -= h_;
		}
	} else if (sym_->id > 0 && sym_->id < 255 &&
		   math_font_available(LM_TC_SYMB)) {
		mathed_char_dim(code2(), size_, sym_->id,
				ascent_, descent_, width_);
	} else {
		mathed_string_dim(LM_TC_TEX, size_, sym_->name, ascent_, descent_, width_);
	}
}


void MathSymbolInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);
	MathTextCodes Code = code();
	if (sym_->latex_font_id > 0 && math_font_available(Code))
		drawChar(pain, Code, size_, x, y - h_, sym_->latex_font_id);
	else if (sym_->id > 0 && sym_->id < 255 &&
		 math_font_available(LM_TC_SYMB))
		drawChar(pain, code2(), size_, x, y, sym_->id);
	else
		drawStr(pain, LM_TC_TEX, size_, x, y, sym_->name);
}


bool MathSymbolInset::isRelOp() const
{
	return sym_->type == LMB_RELATION;
}


bool MathSymbolInset::isScriptable() const
{
	return sym_->token == LM_TK_CMEX;
}
