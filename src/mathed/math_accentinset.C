#include <config.h>

#include "math_accentinset.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "support/LOstream.h"

using std::ostream;

MathAccentInset::MathAccentInset(int f)
	: MathInset(1), code(f)
{}


MathInset * MathAccentInset::clone() const
{   
	return new MathAccentInset(*this);
}

void MathAccentInset::Metrics(MathStyles st, int, int)
{
	xcell(0).Metrics(st);
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();
	width_   = xcell(0).width();
	dh = 5;

	if (code == LM_not) {
		ascent_  += dh;
		descent_ += dh;
		dh = height();
	} else 
		ascent_ += dh + 2;
	
	dy = ascent_;
}

void MathAccentInset::draw(Painter & pain, int x, int y)
{
	int const dw = width() - 2;
	xcell(0).draw(pain, x, y);
	x += (code == LM_not) ? (width() - dw) / 2 : 2;
	mathed_draw_deco(pain, x, y - dy, dw, dh, code);
}


void MathAccentInset::Write(ostream & os, bool fragile) const
{
	latexkeys const * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << '\\' << l->name;
	if (code == LM_not)
		os << ' ';
	else
		os << '{';
	
	cell(0).Write(os, fragile);
	
	if (code != LM_not)
		os << '}';
}


void MathAccentInset::WriteNormal(ostream & os) const
{
	latexkeys const * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << "[accent " << l->name << " ";

	cell(0).WriteNormal(os);

	os << "] ";
}
