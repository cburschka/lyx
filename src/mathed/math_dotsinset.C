#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathDotsInset::MathDotsInset(string const & name, int id)
	: MathInset(0, name), code_(id)
{}


MathInset * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}     


void MathDotsInset::draw(Painter & pain, int x, int y)
{
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, code_);
	if (code_ == LM_vdots || code_ == LM_ddots)
		++x;
	if (code_ != LM_vdots)
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, code_);
}


void MathDotsInset::Metrics(MathStyles st)
{
	size(st);
	mathed_char_dim(LM_TC_VAR, size(), 'M', ascent_, descent_, width_);
	switch (code_) {
		case LM_ldots: dh_ = 0; break;
		case LM_cdots: dh_ = ascent_/2; break;
		case LM_vdots: width_ /= 2;
		case LM_ddots: dh_ = ascent_; break;
	}
} 


void MathDotsInset::Write(ostream & os, bool /* fragile */) const
{
	os << '\\' << name() << ' ';
}


void MathDotsInset::WriteNormal(ostream & os) const
{
	os << "[" << name() << "] ";
}
