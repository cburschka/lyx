#include <config.h>

#include "math_accentinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"

using std::ostream;

MathAccentInset::MathAccentInset(byte cx, MathedTextCodes f, int cd, short st)
	: MathedInset("", LM_OT_ACCENT, st), c(cx), fn(f), code(cd)
{
	inset = 0;
}


MathAccentInset::MathAccentInset(MathedInset *ins, int cd, short st)
	: MathedInset("", LM_OT_ACCENT, st),
	  c(0), fn(LM_TC_MIN), code(cd), inset(ins) {}


MathAccentInset::~MathAccentInset()
{
	delete inset;
}


MathedInset * MathAccentInset::Clone()
{   
	MathAccentInset * p;
	
	if (inset) 
		p = new MathAccentInset(inset->Clone(), code, GetStyle());
	else
		p = new MathAccentInset(c, fn, code, GetStyle());
	
	return p;
}


void
MathAccentInset::draw(Painter & pain, int x, int y)
{
	int dw = width - 2;
	
	if (inset) 
		inset->draw(pain, x, y);
	else {
		string s;
		s += c;
		drawStr(pain, fn, size(), x, y, s);
	}
	x += (code == LM_not) ? (width-dw) / 2 : 2;
	mathed_draw_deco(pain, x, y - dy, dw, dh, code);
}


void
MathAccentInset::Metrics()
{
	if (inset) {
		inset->Metrics();
		ascent = inset->Ascent();
		descent = inset->Descent();
		width = inset->Width();
		dh = ascent;
	} else {
		mathed_char_height(fn, size(), c, ascent, descent);
		width = mathed_char_width(fn, size(), c);
		dh = (width - 2) / 2; 
	}
	if (code == LM_not) {
		ascent += dh;
		descent += dh;
		dh = Height();
	} else 
		ascent += dh+2;
	
	dy = ascent;
//    if (MathIsBinary(fn))
//	width += 2*mathed_char_width(fn, size, ' ');    
}


void MathAccentInset::Write(ostream & os, bool fragile)
{
	latexkeys const * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << '\\' << l->name;
	if (code!= LM_not)
		os << '{';
	else
		os << ' ';
	
	if (inset) {
		inset->Write(os, fragile);
	} else {
		if (fn>= LM_TC_RM && fn <= LM_TC_TEXTRM) {
			os << '\\'
			   << math_font_name[fn - LM_TC_RM]
			   << '{';
		}
		if (MathIsSymbol(fn)) {
			latexkeys const * l = lm_get_key_by_id(c, LM_TK_SYM);
			if (l) {
				os << '\\' << l->name << ' ';
			}
		} else
			os << char(c);
		
		if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM)
			os << '}';
	}
	
	if (code!= LM_not)
		os << '}';
}
