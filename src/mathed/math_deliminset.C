#ifdef __GNUG__
#pragma implementation
#endif

#include "math_deliminset.h"
#include "math_parser.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


MathDelimInset::MathDelimInset(latexkeys const * l, latexkeys const * r)
	: MathNestInset(1), left_(l), right_(r)
{}


MathInset * MathDelimInset::clone() const
{   
	return new MathDelimInset(*this);
}


string MathDelimInset::latexName(latexkeys const * l)
{
	//static const string vdelim("(){}[]./|");
	string name = l->name;
	if (name == "(")
		return name;
	if (name == "[")
		return name;
	if (name == ".")
		return name;
	if (name == ")")
		return name;
	if (name == "]")
		return name;
	if (name == "/")
		return name;
	if (name == "|")
		return name;
	return "\\" + name + " ";
}


void MathDelimInset::write(std::ostream & os, bool fragile) const
{
	os << "\\left" << latexName(left_);
	cell(0).write(os, fragile);
	os << "\\right" << latexName(right_);
}


void MathDelimInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(y); 

	int w = dw();
	xcell(0).draw(pain, x + w, y);
	
	if (latexName(left_) == ".") {
		pain.line(x + 4, yo() - ascent_, x + 4, yo() + descent_,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent_, w, height(), left_);

	x += width() - w - 2;

	if (latexName(right_) == ".") {
		pain.line(x + 4, yo() - ascent_, x + 4, yo() + descent_,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent_, w, height(), right_);
}


int MathDelimInset::dw() const
{
	int w = height()/5;
	if (w > 15)
		w = 15;
	if (w < 6)
		w = 6;
	return w;
}


void MathDelimInset::metrics(MathStyles st) const
{
	xcell(0).metrics(st);
	size_    = st;
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
	width_   = xcell(0).width() + 2 * dw() + 4;
}
