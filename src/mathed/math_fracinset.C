#include <config.h>

#include "math_fracinset.h"
#include "math_iter.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathFracInset::MathFracInset(short ot)
	: MathParInset(LM_ST_TEXT, "frac", ot)
{
	
	den = new MathParInset(LM_ST_TEXT); // this leaks
	dh = 0;
	idx = 0;
	if (objtype == LM_OT_STACKREL) {
		flag |= LMPF_SCRIPT;
		SetName("stackrel");
	}
}


MathFracInset::~MathFracInset()
{
	delete den;
}


MathedInset * MathFracInset::Clone()
{   
	MathFracInset * p = new MathFracInset(GetType());
	MathedIter itn(array);
	MathedIter itd(den->GetData());
	p->SetData(itn.Copy(), itd.Copy());
	p->idx = idx;
	p->dh = dh;
	return p;
}


bool MathFracInset::setArgumentIdx(int i)
{
	if (i == 0 || i == 1) {
		idx = i;
		return true;
	} else 
		return false;
}


void MathFracInset::SetStyle(short st)
{
	MathParInset::SetStyle(st);
	dh = 0;
	den->SetStyle((size == LM_ST_DISPLAY) ?
		      static_cast<short>(LM_ST_TEXT)
		      : size);
}


void MathFracInset::SetData(MathedArray * n, MathedArray * d)
{
	den->SetData(d);
	MathParInset::SetData(n);
}


void MathFracInset::SetData(MathedArray * d)
{
	if (idx == 0)
		MathParInset::SetData(d);
	else {
		den->SetData(d);
	}
}


void MathFracInset::GetXY(int & x, int & y) const
{  
	if (idx == 0)
		MathParInset::GetXY(x, y);
	else
		den->GetXY(x, y);
}


MathedArray * MathFracInset::GetData()
{
	if (idx == 0)
		return array;
	else
		return den->GetData();
}


bool MathFracInset::Inside(int x, int y) 
{
	int xx = xo - (width - w0) / 2;
	
	return x >= xx && x <= xx + width && y <= yo + descent && y >= yo - ascent;
}


void MathFracInset::SetFocus(int /*x*/, int y)
{  
//    lyxerr << "y " << y << " " << yo << " " << den->yo << " ";
	idx = (y > yo) ? 1: 0;
}


void
MathFracInset::draw(Painter & pain, int x, int y)
{ 
	short idxp = idx;
	short sizex = size;
	
	idx = 0;
	if (size == LM_ST_DISPLAY) ++size;
	MathParInset::draw(pain, x + (width - w0) / 2, y - des0);
	den->draw(pain, x + (width - w1) / 2, y + den->Ascent() + 2 - dh);
	size = sizex;
	if (objtype == LM_OT_FRAC)
		pain.line(x + 2, y - dh, x + width - 4, y - dh, LColor::mathline);
	idx = idxp;
}


void
MathFracInset::Metrics()
{
	if (!dh) {
		int a, b;
		dh = mathed_char_height(LM_TC_CONST, size, 'I', a, b)/2;
	}
	short idxp = idx;
	short sizex = size; 
	idx = 0;
	if (size == LM_ST_DISPLAY) ++size; 
	MathParInset::Metrics();
	size = sizex;
	w0 = width;
	int as = Height() + 2 + dh;
	des0 = Descent() + 2 + dh;
	den->Metrics();  
	w1 = den->Width();   
	width = ((w0 > w1) ? w0: w1) + 12;
	ascent = as; 
	descent = den->Height()+ 2 - dh;
	idx = idxp;
}


void MathFracInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile);
	os << "}{";
	den->Write(os, fragile);
	os << '}';
}
