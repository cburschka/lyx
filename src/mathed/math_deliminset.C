#ifdef __GNUG__
#pragma implementation
#endif

#include "math_deliminset.h"
#include "math_parser.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


MathDelimInset::MathDelimInset(int l, int r)
	: MathInset(1), left_(l), right_(r)
{}


MathInset * MathDelimInset::clone() const
{   
	return new MathDelimInset(*this);
}


void MathDelimInset::write(std::ostream & os, bool fragile) const
{
	latexkeys const * l = (left_ != '|') ?
		lm_get_key_by_id(left_, LM_TK_SYM) : 0;
	latexkeys const * r = (right_ != '|') ?
		lm_get_key_by_id(right_, LM_TK_SYM) : 0;

	os << "\\left";
	if (l)
		os << '\\' << l->name << ' ';
	else {
		if (left_ == '{' || left_ == '}')
			os << '\\' << char(left_) << ' ';
		else
			os << char(left_) << ' ';
	}

	cell(0).write(os, fragile);

	os << "\\right";
	if (r)
		os << '\\' << r->name << ' ';
	else {
		if (right_ == '{' || right_ == '}')
			os << '\\' << char(right_) << ' ';
		else
			os << char(right_) << ' ';
	}
}


void MathDelimInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y); 

	int w = dw();
	xcell(0).draw(pain, x + w, y);
	
	if (left_ == '.') {
		pain.line(x + 4, yo() - ascent_, x + 4, yo() + descent_,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent_, w, height(), left_);

	x += width() - w - 2;

	if (right_ == '.') {
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


void MathDelimInset::metrics(MathStyles st)
{
	xcell(0).metrics(st);
	size_    = st;
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
	width_   = xcell(0).width() + 2 * dw() + 4;
}
