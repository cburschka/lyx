#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_deliminset.h"
#include "math_parser.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::max;


MathDelimInset::MathDelimInset(string const & l, string const & r)
	: MathNestInset(1), left_(l), right_(r)
{}


MathInset * MathDelimInset::clone() const
{   
	return new MathDelimInset(*this);
}


string MathDelimInset::latexName(string const & name)
{
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


void MathDelimInset::write(MathWriteInfo & os) const
{
	os << "\\left" << latexName(left_) << cell(0)
	   << "\\right" << latexName(right_);
}


void MathDelimInset::writeNormal(std::ostream & os) const
{
	os << "[delim " << latexName(left_) << ' ' << latexName(right_) << ' ';
	cell(0).writeNormal(os);
	os << "]";
}


int MathDelimInset::dw() const
{
	int w = height() / 5;
	if (w > 15)
		w = 15;
	if (w < 4)
		w = 4;
	return w;
}


void MathDelimInset::metrics(MathMetricsInfo const & mi) const
{
	xcell(0).metrics(mi);
	int a, d, w;
	mathed_char_dim(LM_TC_VAR, mi, 'I', a, d, w);
	int h0   = (a + d) / 2;
	int a0   = std::max(xcell(0).ascent(), a)   - h0;
	int d0   = std::max(xcell(0).descent(), d)  + h0;
	ascent_  = max(a0, d0) + h0;
	descent_ = max(a0, d0) - h0;
	width_   = xcell(0).width() + 2 * dw() + 4;
}


void MathDelimInset::draw(Painter & pain, int x, int y) const
{ 
	int const w = dw();
	int const b = y - ascent_;
	xcell(0).draw(pain, x + w + 2, y);
	mathed_draw_deco(pain, x + 1, b, w, height(), left_);
	mathed_draw_deco(pain, x + width() - w - 1, b, w, height(), right_);
}


string MathDelimInset::octavize() const
{
	if (left_ == "|" && right_ == "|")
		return "det(" + cell(0).octavize() + ")";
	return left_ + cell(0).octavize() + right_;
}


string MathDelimInset::maplize() const
{
	if (left_ == "|" && right_ == "|")
		return "abs(" + cell(0).octavize() + ")";
	return left_ + cell(0).octavize() + right_;
}
