#include <config.h>

#include "math_symbolinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "debug.h"


using std::ostream;


MathSymbolInset::MathSymbolInset(const latexkeys * l)
	: sym_(l), h_(0)
{}


MathSymbolInset::MathSymbolInset(const char * name)
	: sym_(in_word_set(name)), h_(0)
{}


MathSymbolInset::MathSymbolInset(string const & name)
	: sym_(in_word_set(name.c_str())), h_(0)
{}



MathInset * MathSymbolInset::clone() const
{
	return new MathSymbolInset(*this);
}


MathTextCodes MathSymbolInset::code() const
{
	switch (sym_->token) {
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


string MathSymbolInset::name() const
{
	return sym_->name;
}


void MathSymbolInset::metrics(MathMetricsInfo const & mi) const
{
	LyXFont font;
	mi_ = mi;
	MathTextCodes c = code();
	if (sym_->latex_font_id > 0 && math_font_available(c)) {
		whichFont(font, c, mi_);
		mathed_char_dim(font, char(sym_->latex_font_id), ascent_, descent_, width_);
		if (c == LM_TC_CMEX) {
			h_ = 4 * descent_ / 5;
			ascent_  += h_;
			descent_ -= h_;
		}
	} else {
		if (sym_->id > 0 && sym_->id < 255 && math_font_available(LM_TC_SYMB)) {
			whichFont(font, code2(), mi_);
			mathed_char_dim(font, char(sym_->id), ascent_, descent_, width_);
		} else {
			whichFont(font, LM_TC_TEX, mi_);
			mathed_string_dim(font, sym_->name, ascent_, descent_, width_);
		}
	}
	if (isRelOp())
		width_ += 6;
}


void MathSymbolInset::draw(Painter & pain, int x, int y) const
{  
	if (isRelOp())
		x += 3;
	MathTextCodes Code = code();
	LyXFont font;
	if (sym_->latex_font_id > 0 && math_font_available(Code)) {
		whichFont(font, Code, mi_);
		drawChar(pain, font, x, y - h_, char(sym_->latex_font_id));
	} else if (sym_->id > 0 && sym_->id < 255 && math_font_available(LM_TC_SYMB)){
		whichFont(font, code2(), mi_);
		drawChar(pain, font, x, y, char(sym_->id));
	} else {
		whichFont(font, LM_TC_TEX, mi_);
		drawStr(pain, font, x, y, sym_->name);
	}
}


bool MathSymbolInset::isRelOp() const
{	
	return sym_->type == "mathrel";
}


bool MathSymbolInset::isScriptable() const
{
	return mi_.style == LM_ST_DISPLAY && sym_->token == LM_TK_CMEX;
}


bool MathSymbolInset::takesLimits() const
{
	return sym_->token == LM_TK_CMEX;
}

void MathSymbolInset::validate(LaTeXFeatures & features) const
{
	if (sym_->token == LM_TK_MSA || sym_->token == LM_TK_MSB)
		features.require("amssymb");
}

void MathSymbolInset::normalize(NormalStream & os) const
{
	os << "[symbol " << name() << "]";
}


void MathSymbolInset::maplize(MapleStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else
		os << name();
}


char const * MathMLtype(string const & s)
{
	if (s == "mathop")
		return "mo";
	return "mi";
}


bool MathSymbolInset::match(MathInset * p) const
{
	MathSymbolInset const * q = p->asSymbolInset();
	return q && name() == q->name();
}


void MathSymbolInset::mathmlize(MathMLStream & os) const
{
	char const * type = MathMLtype(sym_->type);
	os << '<' << type << "> ";
	if (sym_->xmlname == "x") // unknown so far
		os << name();
	else
		os << sym_->xmlname;
	os << " </" << type << '>';
}


void MathSymbolInset::octavize(OctaveStream & os) const
{
	if (name() == "cdot")
		os << '*';
	else
		os << name();
}


void MathSymbolInset::write(WriteStream & os) const
{
	os << '\\' << name() << ' ';
}


void MathSymbolInset::infoize(ostream & os) const
{
	os << '\\' << name();
}


