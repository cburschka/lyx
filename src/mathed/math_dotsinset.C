#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathDotsInset::MathDotsInset(string const & nam, int id, short st)
	: MathedInset(nam, LM_OT_DOTS, st), code_(id) {}


MathedInset * MathDotsInset::Clone()
{
	return new MathDotsInset(name, code_, GetStyle());
}     


void MathDotsInset::draw(Painter & pain, int x, int y)
{
	mathed_draw_deco(pain, x + 2, y - dh_, width - 2, ascent, code_);
	if (code_ == LM_vdots || code_ == LM_ddots)
		++x;
	if (code_ != LM_vdots)
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width - 2, ascent, code_);
}


void MathDotsInset::Metrics()
{
	mathed_char_height(LM_TC_VAR, size(), 'M', ascent, descent);
	width = mathed_char_width(LM_TC_VAR, size(), 'M');   
	switch (code_) {
		case LM_ldots: dh_ = 0; break;
		case LM_cdots: dh_ = ascent/2; break;
		case LM_vdots: width /= 2;
		case LM_ddots: dh_ = ascent; break;
	}
} 


void MathDotsInset::Write(ostream & os, bool /* fragile */)
{
	os << '\\' << name << ' ';
}


void MathDotsInset::WriteNormal(ostream & os)
{
	os << "[" << name << "] ";
}
