#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_deliminset.h"
#include "math_parser.h"
#include "support.h"
#include "support/LOstream.h"
#include "math_mathmlstream.h"


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
	os << "\\left" << latexName(left_).c_str() << cell(0)
	   << "\\right" << latexName(right_).c_str();
}


void MathDelimInset::writeNormal(NormalStream & os) const
{
	os << "[delim " << latexName(left_).c_str() << ' '
		<< latexName(right_).c_str() << ' ' << cell(0) << ']';
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
	ascent_  = std::max(a0, d0) + h0;
	descent_ = std::max(a0, d0) - h0;
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


bool MathDelimInset::isMatrix() const
{
	return left_ == "(" && right_ == ")" && cell(0).size() == 1 && 
		cell(0).begin()->nucleus() && cell(0).begin()->nucleus()->asArrayInset();
}


void MathDelimInset::maplize(MapleStream & os) const
{
	if (left_ == "|" && right_ == "|") {
		if (cell(0).isMatrix())	
			os << "linalg[det](" << cell(0) << ")";
		else
			os << "abs(" << cell(0) << ")";
	}
	else
		os << left_.c_str() << cell(0) << right_.c_str();
}


void MathDelimInset::mathmlize(MathMLStream & os) const
{
	os << "<fenced open=\"" << left_.c_str() << "\" close=\""
		<< right_.c_str() << "\">" << cell(0) << "</fenced>";
}


void MathDelimInset::octavize(OctaveStream & os) const
{
	if (left_ == "|" && right_ == "|")
		os << "det(" << cell(0) << ")";
	else
		os << left_.c_str() << cell(0) << right_.c_str();
}
