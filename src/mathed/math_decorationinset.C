#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_decorationinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"
#include "support/lstrings.h"


using std::ostream;


MathDecorationInset::MathDecorationInset(int d)
	: MathInset("", LM_OT_DECO, 1), deco_(d)
{
	upper_ = deco_ != LM_underline && deco_ != LM_underbrace;
}


bool MathDecorationInset::GetLimits() const
{ 
	return deco_ == LM_underbrace || deco_ == LM_overbrace;
}    


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}


void MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
	xcell(0).draw(pain, x, y);
	mathed_draw_deco(pain, x, y + 10, width_, 10, deco_);
}


void MathDecorationInset::Metrics(MathStyles st)
{
	int const h = 2 * mathed_char_height(LM_TC_VAR, size(), 'I',
					     ascent_, descent_);  
	xcell(0).Metrics(st);
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();

	int w = width() + 4;
	if (w < 16)
		w = 16;

	int dh_ = w / 5;
	if (dh_ > h)
		dh_ = h;

	int dy_;	
	if (upper_) {
		ascent_ += dh_ + 2;
		dy_ = -ascent_;
	} else {
		dy_ = descent_ + 2;
		descent_ += dh_ + 4;
	}
	width_ = w;
}


void MathDecorationInset::Write(ostream & os, bool fragile) const
{
	latexkeys const * l = lm_get_key_by_id(deco_, LM_TK_WIDE);
	if (fragile &&
	    (compare(l->name, "overbrace") == 0 ||
	     compare(l->name, "underbrace") == 0 ||
	     compare(l->name, "overleftarrow") == 0 ||
	     compare(l->name, "overrightarrow") == 0))
		os << "\\protect";
	os << '\\' << l->name << '{';
	cell(0).Write(os, fragile);  
	os << '}';
}
