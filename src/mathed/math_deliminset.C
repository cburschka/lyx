#include <config.h>

#include "math_deliminset.h"
#include "math_iter.h"
#include "math_parser.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathDelimInset::MathDelimInset(int l, int r, short st)
	: MathParInset(st, "", LM_OT_DELIM), left_(l), right_(r) {}


MathedInset * MathDelimInset::Clone()
{   
	MathDelimInset * p = new MathDelimInset(left_, right_, GetStyle());
	MathedIter it(array);
	p->setData(it.Copy());
	return p;
}


void MathDelimInset::Write(ostream & os, bool fragile)
{
	latexkeys const * l = (left_ != '|') ?
		lm_get_key_by_id(left_, LM_TK_SYM) : 0;
	latexkeys const * r = (right_ != '|') ?
		lm_get_key_by_id(right_, LM_TK_SYM) : 0;
	os << "\\left";
	if (l) {
		os << '\\' << l->name << ' ';
	} else {
		if (left_ == '{' || left_ == '}') {
			os << '\\' << char(left_) << ' ';
		} else {
			os << char(left_) << ' ';
		}
	}
	MathParInset::Write(os, fragile);
	os << "\\right";
	if (r) {
		os << '\\' << r->name << ' ';
	} else {
		if (right_ == '{' || right_ == '}') {
			os << '\\' << char(right_) << ' ';
		} else {
			os << char(right_) << ' ';
		}
	}
}


void
MathDelimInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y); 
	MathParInset::draw(pain, x + dw_ + 2, y - dh_); 
	
	if (left_ == '.') {
		pain.line(x + 4, yo() - ascent,
			  x + 4, yo() + descent,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent, dw_, Height(), left_);
	x += Width() - dw_ - 2;
	if (right_ == '.') {
		pain.line(x + 4, yo() - ascent,
			  x + 4, yo() + descent,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent, dw_, Height(), right_);
}


void
MathDelimInset::Metrics()
{
	MathParInset::Metrics();
	int d;
	
	mathed_char_height(LM_TC_CONST, size(), 'I', d, dh_);
	dh_ /= 2;
	ascent += 2 + dh_;
	descent += 2 - dh_;
	dw_ = Height()/5;
	if (dw_ > 15) dw_ = 15;
	if (dw_ < 6) dw_ = 6;
	width += 2 * dw_ + 4;
}
