#include <config.h>

#include "math_bigopinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;

MathBigopInset::MathBigopInset(string const & name, int id)
	: MathInset(name, LM_OT_BIGOP), lims_(-1), sym_(id)
{}


MathInset * MathBigopInset::Clone() const
{
	return new MathBigopInset(*this);
}



void MathBigopInset::Write(ostream & os, bool fragile) const
{
	bool const limp = GetLimits();
	
	os << '\\' << name();

	bool f = sym_ != LM_int && sym_ != LM_oint && size() == LM_ST_DISPLAY;
	
	if (limp && !f)
		os << "\\limits ";
	else 
		if (!limp && f)
			os << "\\nolimits ";
		else 
			os << ' ';
}


void MathBigopInset::WriteNormal(ostream & os) const
{
	bool const limp = GetLimits();
	bool f = sym_ != LM_int && sym_ != LM_oint;
	
	os << "[bigop " << name();
	
	if (limp && !f)
		os << " limits";
	else 
		if (!limp && f)
			os << " nolimits";
	
	os << "] ";
}

void MathBigopInset::Metrics(MathStyles st)
{
	size(st);
	string s;
	short t;
	
	if (sym_ < 256 || sym_ == LM_oint) {
		char const c = (sym_ == LM_oint) ? LM_int : sym_;
		s += c;
		t = LM_TC_BSYM;
	} else {
		s = name();
		t = LM_TC_TEXTRM;
	}
	mathed_string_dim(t, size(), s, ascent_, descent_, width_);
	if (sym_ == LM_oint)
		width_ += 2;
}


void MathBigopInset::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);
	string s;
	short t;
	
	if (sym_ < 256 || sym_ == LM_oint) {
		s += (sym_ == LM_oint) ? LM_int : sym_;
		t = LM_TC_BSYM;
	} else {
		s = name();
		t = LM_TC_TEXTRM;
	}
	if (sym_ == LM_oint) {
		pain.arc(x, y - 5 * width_ / 4, width_, width_, 0, 360 * 64,
			 LColor::mathline);
		++x;
	}
	pain.text(x, y, s, mathed_get_font(t, size()));
}


bool MathBigopInset::GetLimits() const 
{  
	// Default case
	if (lims_ < 0) 
		return sym_ != LM_int && sym_ != LM_oint && size() == LM_ST_DISPLAY;
	
	// Custom 
	return lims_ > 0;
} 


void MathBigopInset::SetLimits(bool ls) 
{  
	lims_ = ls ? 1 : 0; 
}
