#include <config.h>

#include <functional>

#include "math_bigopinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;

MathBigopInset::MathBigopInset(string const & name, int id)
	: MathScriptInset(true, false), lims_(0), sym_(id)
{
	SetName(name);
}


MathInset * MathBigopInset::clone() const
{
	return new MathBigopInset(*this);
}



void MathBigopInset::Write(ostream & os, bool fragile) const
{
	//bool f = sym_ != LM_int && sym_ != LM_oint && size() == LM_ST_DISPLAY;
	os << '\\' << name();
	if (limits() == 1)
		os << "\\limits ";
	else if (limits() == -1)
		os << "\\nolimits ";
	else 
		os << ' ';
	MathScriptInset::Write(os, fragile);
}


void MathBigopInset::WriteNormal(ostream & os) const
{
	os << "[bigop " << name();
	if (limits() == 1)
		os << "\\limits ";
	else if (limits() == -1)
		os << "\\nolimits ";
	else 
		os << ' ';
	MathScriptInset::WriteNormal(os);
	os << "] ";
}

void MathBigopInset::Metrics(MathStyles st)
{
	MathScriptInset::Metrics(st);
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

	int asc, des, wid;
	mathed_string_dim(t, size(), s, asc, des, wid);
	if (sym_ == LM_oint)
		wid += 2;

	if (hasLimits()) {
		ascent_  = asc + xcell(0).height() + 2;
		descent_ = des + xcell(1).height() + 2;
		width_   = std::max(width_, wid);
	} else {
		ascent_  = std::max(ascent_, asc);
		descent_ = std::max(descent_, des);
		width_  += wid;
	}

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

	int asc, des, wid;
	mathed_string_dim(t, size(), s, asc, des, wid);

	if (hasLimits()) {
		int w = width();
		pain.text(x + (w - wid)/2, y, s, mathed_get_font(t, size()));
		xcell(0).draw
			(pain, x + (w - xcell(0).width())/2, y - asc - xcell(0).descent() - 1);
		xcell(1).draw
			(pain, x + (w - xcell(1).width())/2, y + des + xcell(1).ascent()  + 1);
	} else {
		pain.text(x, y, s, mathed_get_font(t, size()));
		MathScriptInset::draw(pain, x + wid, y);
	}
}


int MathBigopInset::limits() const 
{
	return lims_;	
} 


void MathBigopInset::limits(int limit) 
{  
	lims_ = limit;
}

bool MathBigopInset::hasLimits() const
{
	return limits() == 1 || (limits() == 0 && size() == LM_ST_DISPLAY);
}


bool MathBigopInset::idxDelete(int idx)
{
	// ignore the return value, we do not want the inset to be deleted
	MathScriptInset::idxDelete(idx);
	return false;
}
