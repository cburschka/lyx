#ifdef __GNUG__
#pragma implementation
#endif

#include "math_charinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "debug.h"


MathCharInset::MathCharInset(char c, MathTextCodes t)
	: char_(c)
{
	code(t);
}


MathInset * MathCharInset::clone() const
{   
	return new MathCharInset(*this);
}


int MathCharInset::ascent() const
{
	return mathed_char_ascent(code_, size(), char_);
}


int MathCharInset::descent() const
{
	return mathed_char_descent(code_, size(), char_);
}


int MathCharInset::width() const
{
	return mathed_char_width(code_, size(), char_);
}


void MathCharInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y);
	drawChar(pain, code_, size_, x, y, char_);
}


void MathCharInset::write(std::ostream & os, bool) const
{
	int brace = 0;

	if (MathIsSymbol(code_)) {
		latexkeys const * l = lm_get_key_by_id(char_, LM_TK_SYM);

		if (l == 0) 
			l = lm_get_key_by_id(char_, LM_TK_BIGSYM);

		if (l) {
			os << '\\' << l->name << ' ';
		} else {
			lyxerr << "Could not find the LaTeX name for  "	
				<< char_ << " and code_ " << code_ << "!" << std::endl;
		}
	} else {
		if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM) 
			os << '\\' << math_font_name[code_ - LM_TC_RM] << '{';

		// Is there a standard logical XOR?
		if ((code_ == LM_TC_TEX && char_ != '{' && char_ != '}') ||
				(code_ == LM_TC_SPECIAL))
			os << '\\';
		else {
			if (char_ == '{')
				++brace;
			if (char_ == '}')
				--brace;
		}
		if (char_ == '}' && code_ == LM_TC_TEX && brace < 0) 
			lyxerr <<"Math warning: Unexpected closing brace.\n";
		else	       
			os << char_;
	}

	if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM)
		os << '}';
}


void MathCharInset::writeNormal(std::ostream & os) const
{
	os << "[sqrt ";
	cell(0).writeNormal(os); 
	os << "] ";
}
