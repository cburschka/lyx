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
	: MathInset(1), deco_(d)
{
	upper_ = deco_ != LM_underline && deco_ != LM_underbrace;
}


MathInset * MathDecorationInset::clone() const
{   
	return new MathDecorationInset(*this);
}



void MathDecorationInset::Metrics(MathStyles st)
{
	xcell(0).Metrics(st);
	size_    = st;
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();

	dh_ = 5; //mathed_char_height(LM_TC_VAR, size(), 'I', ascent_, descent_);  

	if (upper_) {
		dy_ = -ascent_ - dh_;
		ascent_ += dh_ + 1;
	} else {
		dy_ = descent_ + 1;
		descent_ += dh_ + 2;
	}
}

void MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(x);
	xcell(0).draw(pain, x, y);
	mathed_draw_deco(pain, x, y + dy_, width_, dh_, deco_);
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
