#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

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
	
	den_ = new MathParInset(LM_ST_TEXT); // this leaks
	dh_ = 0;
	idx_ = 0;
	if (objtype == LM_OT_STACKREL) {
		flag |= LMPF_SCRIPT;
		SetName("stackrel");
	}
}


MathFracInset::~MathFracInset()
{
	delete den_;
}


MathedInset * MathFracInset::Clone()
{   
	MathFracInset * p = new MathFracInset(*this);
	// this cast will go again...
	p->den_ = static_cast<MathParInset*>(p->den_->Clone());
	return p;
}


bool MathFracInset::setArgumentIdx(int i)
{
	if (i == 0 || i == 1) {
		idx_ = i;
		return true;
	} else 
		return false;
}


void MathFracInset::SetStyle(short st)
{
	MathParInset::SetStyle(st);
	dh_ = 0;
	den_->SetStyle((size() == LM_ST_DISPLAY) ?
		      static_cast<short>(LM_ST_TEXT)
		      : size());
}


void MathFracInset::SetData(MathedArray const & n, MathedArray const & d)
{
	den_->setData(d);
	MathParInset::setData(n);
}


void MathFracInset::setData(MathedArray const & d)
{
	if (idx_ == 0)
		MathParInset::setData(d);
	else {
		den_->setData(d);
	}
}


void MathFracInset::GetXY(int & x, int & y) const
{  
	if (idx_ == 0)
		MathParInset::GetXY(x, y);
	else
		den_->GetXY(x, y);
}


MathedArray & MathFracInset::GetData()
{
	if (idx_ == 0)
		return array;
	else
		return den_->GetData();
}


bool MathFracInset::Inside(int x, int y) 
{
	int const xx = xo() - (width - w0_) / 2;
	
	return x >= xx
		&& x <= xx + width
		&& y <= yo() + descent
		&& y >= yo() - ascent;
}


void MathFracInset::SetFocus(int /*x*/, int y)
{  
	// lyxerr << "y " << y << " " << yo << " " << den_->yo << " ";
	idx_ = (y > yo()) ? 1 : 0;
}


void
MathFracInset::draw(Painter & pain, int x, int y)
{ 
	int const idxp = idx_;
	int const sizex = size();
	
	idx_ = 0;
	if (size() == LM_ST_DISPLAY) incSize();
	MathParInset::draw(pain, x + (width - w0_) / 2, y - des0_);
	den_->draw(pain, x + (width - w1_) / 2, y + den_->Ascent() + 2 - dh_);
	size(sizex);
	if (objtype == LM_OT_FRAC)
		pain.line(x + 2, y - dh_,
			  x + width - 4, y - dh_, LColor::mathline);
	idx_ = idxp;
}


void
MathFracInset::Metrics()
{
	if (!dh_) {
		int a;
		int b;
		dh_ = mathed_char_height(LM_TC_CONST, size(), 'I', a, b) / 2;
	}
	int const idxp = idx_;
	int const sizex = size();
	idx_ = 0;
	if (size() == LM_ST_DISPLAY) incSize(); 
	MathParInset::Metrics();
	size(sizex);
	w0_ = width;
	int const as = Height() + 2 + dh_;
	des0_ = Descent() + 2 + dh_;
	den_->Metrics();  
	w1_ = den_->Width();   
	width = ((w0_ > w1_) ? w0_: w1_) + 12;
	ascent = as; 
	descent = den_->Height()+ 2 - dh_;
	idx_ = idxp;
}


void MathFracInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile);
	os << "}{";
	den_->Write(os, fragile);
	os << '}';
}
