#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_deliminset.h"
#include "math_parser.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_extern.h"


using std::max;


MathDelimInset::MathDelimInset(string const & l, string const & r)
	: MathNestInset(1), left_(l), right_(r)
{}


MathInset * MathDelimInset::clone() const
{   
	return new MathDelimInset(*this);
}


void MathDelimInset::write(WriteStream & os) const
{
	os << "\\left" << convertDelimToLatexName(left_) << cell(0)
	   << "\\right" << convertDelimToLatexName(right_);
}


void MathDelimInset::normalize(NormalStream & os) const
{
	os << "[delim " << convertDelimToLatexName(left_) << ' '
	   << convertDelimToLatexName(right_) << ' ' << cell(0) << ']';
}


int MathDelimInset::dw() const
{
	int w = height() / 5;
	if (w > 8)
		w = 8;
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
	int a0   = max(xcell(0).ascent(), a)   - h0;
	int d0   = max(xcell(0).descent(), d)  + h0;
	ascent_  = max(a0, d0) + h0;
	descent_ = max(a0, d0) - h0;
	width_   = xcell(0).width() + 2 * dw() + 8;
}


void MathDelimInset::draw(Painter & pain, int x, int y) const
{ 
	int const w = dw();
	int const b = y - ascent_;
	xcell(0).draw(pain, x + w + 4, y);
	mathed_draw_deco(pain, x + 4, b, w, height(), left_);
	mathed_draw_deco(pain, x + width() - w - 4, b, w, height(), right_);
}


bool MathDelimInset::isParanthesis() const
{
	return left_ == "(" && right_ == ")";
}


bool MathDelimInset::isBrackets() const
{
	return left_ == "[" && right_ == "]";
}


bool MathDelimInset::isAbs() const
{
	return left_ == "|" && right_ == "|";
}


void MathDelimInset::maplize(MapleStream & os) const
{
	if (isAbs()) {
		bool mat =
			cell(0).size() == 1 && cell(0).begin()->nucleus()
					&& cell(0).begin()->nucleus()->asMatrixInset();
		if (mat)	
			os << "linalg[det](" << cell(0) << ")";
		else
			os << "abs(" << cell(0) << ")";
	}
	else
		os << left_ << cell(0) << right_;
}


void MathDelimInset::mathmlize(MathMLStream & os) const
{
	os << "<fenced open=\"" << left_ << "\" close=\""
		<< right_ << "\">" << cell(0) << "</fenced>";
}


void MathDelimInset::octavize(OctaveStream & os) const
{
	if (isAbs())
		os << "det(" << cell(0) << ")";
	else
		os << left_ << cell(0) << right_;
}
