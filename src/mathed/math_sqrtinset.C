#include <config.h>

#include "math_sqrtinset.h"
#include "math_iter.h"
#include "LColor.h"
#include "Painter.h"
#include "support.h"
#include "support/LOstream.h"

using std::ostream;

MathSqrtInset::MathSqrtInset(short st)
	: MathParInset(st, "sqrt", LM_OT_SQRT) {}


MathedInset * MathSqrtInset::Clone()
{   
	MathSqrtInset * p = new MathSqrtInset(GetStyle());
	MathedIter it(array);
	p->setData(it.Copy());
	return p;
}


bool MathSqrtInset::Inside(int x, int y) 
{
	return x >= xo() - hmax_
		&& x <= xo() + width - hmax_
		&& y <= yo() + descent
		&& y >= yo() - ascent;
}


void
MathSqrtInset::draw(Painter & pain, int x, int y)
{ 
	MathParInset::draw(pain, x + hmax_ + 2, y); 
	int const h = ascent;
	int const d = descent;
	int const h2 = Height() / 2;
	int const w2 = (Height() > 4 * hmax_) ? hmax_ : hmax_ / 2; 
	int xp[4];
	int yp[4];
	xp[0] = x + hmax_ + wbody_; yp[0] = y - h;
	xp[1] = x + hmax_;          yp[1] = y - h;
	xp[2] = x + w2;             yp[2] = y + d;
	xp[3] = x;                  yp[3] = y + d - h2;
	pain.lines(xp, yp, 4, LColor::mathline);
}


void
MathSqrtInset::Metrics()
{
	MathParInset::Metrics();
	ascent += 4;
	descent += 2;
	int a;
	int b;
	hmax_ = mathed_char_height(LM_TC_VAR, size(), 'I', a, b);
	if (hmax_ < 10) hmax_ = 10;
	wbody_ = width + 4;
	width += hmax_ + 4;
}


void MathSqrtInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile); 
	os << '}';
}
