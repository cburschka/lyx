#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "math_charinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "debug.h"


namespace {

char const * math_font_name[] = {
	"mathrm",
	"mathcal",
	"mathbf",
	"mathbb",
	"mathsf",
	"mathtt",
	"mathit",
	"textrm"
};

}


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


MathTextCodes MathCharInset::nativeCode(char c) const
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


int MathCharInset::ascent() const
{
	return mathed_char_ascent(code_, mi_, char_);
}


int MathCharInset::descent() const
{
	return mathed_char_descent(code_, mi_, char_);
}


int MathCharInset::width() const
{
	return mathed_char_width(code_, mi_, char_);
}


void MathCharInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
}


void MathCharInset::draw(Painter & pain, int x, int y) const
{ 
	//lyxerr << "drawing '" << char_ << "' code: " << code_ << endl;
	drawChar(pain, code_, mi_, x, y, char_);
}


void MathCharInset::writeHeader(std::ostream & os) const
{
	if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM) 
		os << '\\' << math_font_name[code_ - LM_TC_RM] << '{';
}


void MathCharInset::writeTrailer(std::ostream & os) const
{
	if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM)
		os << '}';
}


void MathCharInset::writeRaw(std::ostream & os) const
{
	os << char_;
}


void MathCharInset::write(MathWriteInfo & os) const
{
	writeHeader(os.os);
	writeRaw(os.os);
	writeTrailer(os.os);
}


void MathCharInset::writeNormal(std::ostream & os) const
{
	os << char_;
}


bool MathCharInset::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}


void MathCharInset::handleFont(MathTextCodes t)
{
	code_ = (code_ == t) ? LM_TC_VAR : t;
}
