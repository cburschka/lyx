#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "math_iter.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"

using std::ostream;


MathDecorationInset::MathDecorationInset(int d, short st)
	: MathParInset(st, "", LM_OT_DECO), deco_(d)
{
	upper_ = (deco_ != LM_underline && deco_ != LM_underbrace);
}


bool MathDecorationInset::GetLimits() const
{ 
	return deco_ == LM_underbrace || deco_ == LM_overbrace;
}    


MathedInset * MathDecorationInset::Clone()
{   
	return new MathDecorationInset(*this);
}


void
MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
	MathParInset::draw(pain, x + (width - dw_) / 2, y);
	mathed_draw_deco(pain, x, y + dy_, width, dh_, deco_);
}


void
MathDecorationInset::Metrics()
{
	int const h = 2 * mathed_char_height(LM_TC_VAR, size(), 'I',
					     ascent, descent);  
	MathParInset::Metrics();
	int w = Width() + 4;
	if (w < 16) w = 16;
	dh_ = w / 5;
	if (dh_ > h) dh_ = h;
	
	if (upper_) {
		ascent += dh_ + 2;
		dy_ = -ascent;
	} else {
		dy_ = descent + 2;
		descent += dh_ + 4;
	}
	dw_ = width;
	width = w;
}


void MathDecorationInset::Write(ostream & os, bool fragile)
{
	latexkeys const * l = lm_get_key_by_id(deco_, LM_TK_WIDE);
	if (fragile &&
	    (strcmp(l->name, "overbrace") == 0 ||
	     strcmp(l->name, "underbrace") == 0 ||
	     strcmp(l->name, "overleftarrow") == 0 ||
	     strcmp(l->name, "overrightarrow") == 0))
		os << "\\protect";
	os << '\\' << l->name << '{';
	MathParInset::Write(os, fragile);  
	os << '}';
}
