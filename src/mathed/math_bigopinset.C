#include <config.h>

#include "math_bigopinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;

bool MathBigopInset::GetLimits() const 
{  
	// Default case
	if (lims < 0) {
		return sym != LM_int && sym != LM_oint && GetStyle() == LM_ST_DISPLAY;
	} 
	
	// Custom 
	return lims > 0;
} 


void MathBigopInset::SetLimits(bool ls) 
{  
	lims = ls ? 1 : 0; 
}


MathBigopInset::MathBigopInset(string const & nam, int id, short st)
	: MathedInset(nam, LM_OT_BIGOP, st), sym(id)
{
	lims = -1;
}


MathedInset * MathBigopInset::Clone()
{
	return new MathBigopInset(name, sym, GetStyle());
}


void
MathBigopInset::draw(Painter & pain, int x, int y)
{
	string s;
	short t;
	
	if (sym < 256 || sym == LM_oint) {
		s += (sym == LM_oint) ? LM_int : sym;
		t = LM_TC_BSYM;
	} else {
		s = name;
		t = LM_TC_TEXTRM;
	}
	if (sym == LM_oint) {
		pain.arc(x, y - 5 * width / 4, width, width, 0, 360*64,
			 LColor::mathline);
		++x;
	}
	pain.text(x, y, s, mathed_get_font(t, size));
}


void
MathBigopInset::Metrics()
{
	//char c;
	string s;
	short t;
	
	if (sym < 256 || sym == LM_oint) {
		char c = (sym == LM_oint) ? LM_int: sym;
		s += c;
		t = LM_TC_BSYM;
	} else {
		s = name;
		t = LM_TC_TEXTRM;
	}
	mathed_string_height(t, size, s, ascent, descent);
	width = mathed_string_width(t, size, s);
	if (sym == LM_oint) width += 2;
}


void MathBigopInset::Write(ostream & os, bool /* fragile */)
{
	bool limp = GetLimits();
	
	os << '\\' << name;
	
	if (limp && !(sym != LM_int && sym != LM_oint
		      && (GetStyle() == LM_ST_DISPLAY)))
		os << "\\limits ";
	else 
		if (!limp && (sym != LM_int && sym != LM_oint
			      && (GetStyle() == LM_ST_DISPLAY)))
			os << "\\nolimits ";
		else 
			os << ' ';
}
