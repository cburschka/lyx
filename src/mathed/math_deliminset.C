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
	: MathParInset(st, "", LM_OT_DELIM), left(l), right(r) {}


MathedInset * MathDelimInset::Clone()
{   
	MathDelimInset * p = new MathDelimInset(left, right, GetStyle());
	MathedIter it(array);
	p->SetData(it.Copy());
	return p;
}



void MathDelimInset::Write(ostream & os, bool fragile)
{
	latexkeys * l = (left != '|') ? lm_get_key_by_id(left, LM_TK_SYM): 0;
	latexkeys * r = (right != '|') ? lm_get_key_by_id(right, LM_TK_SYM): 0;
	os << "\\left";
	if (l) {
		os << '\\' << l->name << ' ';
	} else {
		if (left == '{' || left == '}') {
			os << '\\' << char(left) << ' ';
		} else {
			os << char(left) << ' ';
		}
	}
	MathParInset::Write(os, fragile);
	os << "\\right";
	if (r) {
		os << '\\' << r->name << ' ';
	} else {
		if (right == '{' || right == '}') {
			os << '\\' << char(right) << ' ';
		} else {
			os << char(right) << ' ';
		}
	}
}



void
MathDelimInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y); 
	MathParInset::draw(pain, x + dw + 2, y - dh); 
	
	if (left == '.') {
		pain.line(x + 4, yo() - ascent,
			  x + 4, yo() + descent,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent, dw, Height(), left);
	x += Width() - dw - 2;
	if (right == '.') {
		pain.line(x + 4, yo() - ascent,
			  x + 4, yo() + descent,
			  LColor::mathcursor, Painter::line_onoffdash);
	} else
		mathed_draw_deco(pain, x, y - ascent, dw, Height(), right);
}


void
MathDelimInset::Metrics()
{
	MathParInset::Metrics();
	int d;
	
	mathed_char_height(LM_TC_CONST, size(), 'I', d, dh);
	dh /= 2;
	ascent += 2 + dh;
	descent += 2 - dh;
	dw = Height()/5;
	if (dw > 15) dw = 15;
	if (dw < 6) dw = 6;
	width += 2 * dw + 4;
}
