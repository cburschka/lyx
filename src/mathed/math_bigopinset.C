#include <config.h>

#include "math_bigopinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;

MathBigopInset::MathBigopInset(string const & nam, int id, short st)
	: MathedInset(nam, LM_OT_BIGOP, st), lims_(-1), sym_(id)
{}


MathedInset * MathBigopInset::Clone()
{
	return new MathBigopInset(name, sym_, GetStyle());
}


void
MathBigopInset::draw(Painter & pain, int x, int y)
{
	string s;
	short t;
	
	if (sym_ < 256 || sym_ == LM_oint) {
		s += (sym_ == LM_oint) ? LM_int : sym_;
		t = LM_TC_BSYM;
	} else {
		s = name;
		t = LM_TC_TEXTRM;
	}
	if (sym_ == LM_oint) {
		pain.arc(x, y - 5 * width / 4, width, width, 0, 360 * 64,
			 LColor::mathline);
		++x;
	}
	pain.text(x, y, s, mathed_get_font(t, size()));
}


void MathBigopInset::Write(ostream & os, bool /* fragile */)
{
	bool const limp = GetLimits();
	
	os << '\\' << name;
	
	if (limp && !(sym_ != LM_int && sym_ != LM_oint
		      && (GetStyle() == LM_ST_DISPLAY)))
		os << "\\limits ";
	else 
		if (!limp && (sym_ != LM_int && sym_ != LM_oint
			      && (GetStyle() == LM_ST_DISPLAY)))
			os << "\\nolimits ";
		else 
			os << ' ';
}


void MathBigopInset::WriteNormal(ostream & os)
{
	bool const limp = GetLimits();
	
	os << "{bigop " << name;
	
	if (limp && !(sym_ != LM_int && sym_ != LM_oint
		      && (GetStyle() == LM_ST_DISPLAY)))
		os << " limits";
	else 
		if (!limp && (sym_ != LM_int && sym_ != LM_oint
			      && (GetStyle() == LM_ST_DISPLAY)))
			os << " nolimits";
	
	os << "} ";
}

void MathBigopInset::Metrics()
{
	//char c;
	string s;
	short t;
	
	if (sym_ < 256 || sym_ == LM_oint) {
		char const c = (sym_ == LM_oint) ? LM_int: sym_;
		s += c;
		t = LM_TC_BSYM;
	} else {
		s = name;
		t = LM_TC_TEXTRM;
	}
	mathed_string_height(t, size(), s, ascent, descent);
	width = mathed_string_width(t, size(), s);
	if (sym_ == LM_oint) width += 2;
}


bool MathBigopInset::GetLimits() const 
{  
	// Default case
	if (lims_ < 0) {
		return sym_ != LM_int &&
			sym_ != LM_oint &&
			GetStyle() == LM_ST_DISPLAY;
	} 
	
	// Custom 
	return lims_ > 0;
} 


void MathBigopInset::SetLimits(bool ls) 
{  
	lims_ = ls ? 1 : 0; 
}
