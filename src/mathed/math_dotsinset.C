#include <config.h>

#include "math_dotsinset.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathDotsInset::MathDotsInset(string const & nam, int id, short st)
	: MathedInset(nam, LM_OT_DOTS, st), code(id) {}


MathedInset * MathDotsInset::Clone()
{
	return new MathDotsInset(name, code, GetStyle());
}     


void
MathDotsInset::draw(Painter & pain, int x, int y)
{
	mathed_draw_deco(pain, x + 2, y - dh, width - 2, ascent, code);
	if (code == LM_vdots || code == LM_ddots) ++x; 
	if (code != LM_vdots) --y;
	mathed_draw_deco(pain, x + 2, y - dh, width - 2, ascent, code);
}


void
MathDotsInset::Metrics()
{
	mathed_char_height(LM_TC_VAR, size, 'M', ascent, descent);
	width = mathed_char_width(LM_TC_VAR, size, 'M');   
	switch (code) {
	case LM_ldots: dh = 0; break;
	case LM_cdots: dh = ascent/2; break;
	case LM_vdots: width /= 2;
	case LM_ddots: dh = ascent; break;
	}
} 


void
MathDotsInset::Write(ostream & os, bool /* fragile */)
{
	os << '\\' << name << ' ';
}
