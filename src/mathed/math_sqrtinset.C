#include <config.h>

#include "math_sqrtinset.h"
#include "math_iter.h"
#include "LColor.h"
#include "Painter.h"
#include "support.h"


MathSqrtInset::MathSqrtInset(short st)
	: MathParInset(st, "sqrt", LM_OT_SQRT) {}


MathedInset * MathSqrtInset::Clone()
{   
	MathSqrtInset * p = new MathSqrtInset(GetStyle());
	MathedIter it(array);
	p->SetData(it.Copy());
	return p;
}


bool MathSqrtInset::Inside(int x, int y) 
{
	return x >= xo - hmax
		&& x <= xo + width - hmax
		&& y <= yo + descent
		&& y >= yo - ascent;
}


void
MathSqrtInset::draw(Painter & pain, int x, int y)
{ 
	MathParInset::draw(pain, x + hmax + 2, y); 
	int h = ascent;
	int d = descent;
	int h2 = Height() / 2;
	int w2 = (Height() > 4 * hmax) ? hmax : hmax / 2; 
	int xp[4], yp[4];
	xp[0] = x + hmax + wbody; yp[0] = y - h;
	xp[1] = x + hmax;         yp[1] = y - h;
	xp[2] = x + w2;           yp[2] = y + d;
	xp[3] = x;                yp[3] = y + d - h2;
	pain.lines(xp, yp, 4, LColor::mathline);
}


void
MathSqrtInset::Metrics()
{
	MathParInset::Metrics();
	ascent += 4;
	descent += 2;
	int a, b;
	hmax = mathed_char_height(LM_TC_VAR, size, 'I', a, b);
	if (hmax < 10) hmax = 10;
	wbody = width + 4;
	width += hmax + 4;
}


void MathSqrtInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile); 
	os << '}';
}
