#include <config.h>

#include "math_accentinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"

using std::ostream;

MathAccentInset::MathAccentInset(byte cx, MathTextCodes f, int cd)
	: MathInset(1), c(cx), fn(f), code(cd), inset(0)
{}


MathAccentInset::MathAccentInset(MathInset * ins, int cd)
	: MathInset(0), c(0), fn(LM_TC_MIN), code(cd), inset(ins)
{}


MathAccentInset::~MathAccentInset()
{
	delete inset;
}


MathInset * MathAccentInset::clone() const
{   
	MathAccentInset * p;
	
	if (inset) 
		p = new MathAccentInset(inset->clone(), code);
	else
		p = new MathAccentInset(c, fn, code);
	
	return p;
}


void MathAccentInset::draw(Painter & pain, int x, int y)
{
	int const dw = width() - 2;
	
	if (inset) 
		inset->draw(pain, x, y);
	else 
		drawChar(pain, fn, size(), x, y, c);
	x += (code == LM_not) ? (width() - dw) / 2 : 2;
	mathed_draw_deco(pain, x, y - dy, dw, dh, code);
}


void MathAccentInset::Metrics(MathStyles st)
{
	if (inset) {
		inset->Metrics(st);
		ascent_  = inset->ascent();
		descent_ = inset->descent();
		width_   = inset->width();
		dh = ascent_;
	} else {
		mathed_char_dim(fn, size(), c, ascent_, descent_, width_);
		dh = width() / 2 - 1;
	}
	if (code == LM_not) {
		ascent_  += dh;
		descent_ += dh;
		dh = height();
	} else 
		ascent_ += dh + 2;
	
	dy = ascent_;
//    if (MathIsBinary(fn))
//	width += 2*mathed_char_width(fn, size, ' ');    
}


void MathAccentInset::Write(ostream & os, bool fragile) const
{
	latexkeys const * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << '\\' << l->name;
	if (code!= LM_not)
		os << '{';
	else
		os << ' ';
	
	if (inset)
		inset->Write(os, fragile);
	else {
		if (fn>= LM_TC_RM && fn <= LM_TC_TEXTRM)
			os << '\\' << math_font_name[fn - LM_TC_RM] << '{';
		if (MathIsSymbol(fn)) {
			latexkeys const * l = lm_get_key_by_id(c, LM_TK_SYM);
			if (l)
				os << '\\' << l->name << ' ';
		} else
			os << char(c);
		
		if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM)
			os << '}';
	}
	
	if (code!= LM_not)
		os << '}';
}


void MathAccentInset::WriteNormal(ostream & os) const
{
	latexkeys const * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << "[accent " << l->name << " ";

	if (inset)
		inset->WriteNormal(os);
	else {
		if (fn >= LM_TC_RM && fn <= LM_TC_TEXTRM)
			os << "[font " << math_font_name[fn - LM_TC_RM] << "]";
		if (MathIsSymbol(fn)) {
			latexkeys const * l = lm_get_key_by_id(c, LM_TK_SYM);
			if (l) 
				os << "[symbol " << l->name << "] ";
		} else
			os << "[char " << char(c) << "] ";
	}

	os << "] ";
}

int MathAccentInset::getAccentCode() const
{
	return code;
}
