#include <config.h>

#include "math_decorationinset.h"
#include "math_iter.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"

using std::ostream;


bool MathDecorationInset::GetLimits() const
{ 
	return deco == LM_underbrace || deco == LM_overbrace;
}    


MathDecorationInset::MathDecorationInset(int d, short st)
	: MathParInset(st, "", LM_OT_DECO), deco(d)
{
   upper = (deco!= LM_underline && deco!= LM_underbrace);
}


MathedInset * MathDecorationInset::Clone()
{   
   MathDecorationInset * p = new MathDecorationInset(deco, GetStyle());
   MathedIter it(array);
   p->SetData(it.Copy());
   return p;
}


void
MathDecorationInset::draw(Painter & pain, int x, int y)
{ 
	MathParInset::draw(pain, x + (width - dw) / 2, y);
	mathed_draw_deco(pain, x, y + dy, width, dh, deco);
}


void
MathDecorationInset::Metrics()
{
	int h = 2 * mathed_char_height(LM_TC_VAR, size(), 'I',
				       ascent, descent);  
	MathParInset::Metrics();
	int w = Width() + 4;
	if (w < 16) w = 16;
	dh = w / 5;
	if (dh > h) dh = h;
	
	if (upper) {
		ascent += dh + 2;
		dy = -ascent;
	} else {
		dy = descent + 2;
		descent += dh + 4;
	}
	dw = width;
	width = w;
}


void MathDecorationInset::Write(ostream & os, bool fragile)
{
	latexkeys * l = lm_get_key_by_id(deco, LM_TK_WIDE);
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
