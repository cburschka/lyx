#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "math_charinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "math_support.h"
#include "math_parser.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "LaTeXFeatures.h"
#include "textpainter.h"


using std::ostream;


MathCharInset::MathCharInset(char c)
	: char_(c), code_(nativeCode(c))
{
//lyxerr << "creating char '" << char_ << "' with code " << int(code_) << endl;
}


MathCharInset::MathCharInset(char c, MathTextCodes t)
	: char_(c), code_((t == LM_TC_MIN) ? nativeCode(c) : t)
{
//lyxerr << "creating char '" << char_ << "' with code " << int(code_) << endl;
}


MathTextCodes MathCharInset::nativeCode(char c)
{
	if (isalpha(c))
		return LM_TC_VAR;
	//if (strchr("0123456789;:!|[]().,?+/-*<>=", c)
	return LM_TC_CONST;
}


MathInset * MathCharInset::clone() const
{   
	return new MathCharInset(*this);
}


void MathCharInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, code_, mi);
	mathed_char_dim(font_, char_, ascent_, descent_, width_);
}


void MathCharInset::draw(Painter & pain, int x, int y) const
{ 
	//lyxerr << "drawing '" << char_ << "' code: " << code_ << endl;
	drawChar(pain, font_, x, y, char_);
}


void MathCharInset::metrics(TextMetricsInfo const &) const
{
	width_   = 1;
	ascent_  = 1;
	descent_ = 0;
}


void MathCharInset::draw(TextPainter & pain, int x, int y) const
{ 
	lyxerr << "drawing text '" << char_ << "' code: " << code_ << endl;
	pain.draw(x, y, char_);
}


void MathCharInset::writeHeader(ostream & os) const
{
	if (math_font_name(code_))
		os << '\\' << math_font_name(code_) << '{';
}


void MathCharInset::writeTrailer(ostream & os) const
{
	if (math_font_name(code_))
		os << '}';
}


void MathCharInset::writeRaw(ostream & os) const
{
	os << char_;
}


void MathCharInset::write(WriteStream & os) const
{
	writeHeader(os.os());
	writeRaw(os.os());
	writeTrailer(os.os());
}


void MathCharInset::normalize(NormalStream & os) const
{
	os << "[char " << char_ << " " << "mathalpha" << "]";
}


bool MathCharInset::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}


void MathCharInset::handleFont(MathTextCodes t)
{
	code_ = (code_ == t) ? LM_TC_VAR : t;
}


void MathCharInset::validate(LaTeXFeatures & features) const
{
	// Make sure amssymb is put in preamble if Blackboard Bold or
	// Fraktur used:
	if ((code_ == LM_TC_BB) || (code_ == LM_TC_EUFRAK))
		features.require("amssymb");
}


bool MathCharInset::match(MathInset * p) const
{
	MathCharInset const * q = p->asCharInset();
	return q && char_ == q->char_ && code_ == q->code_;
}
