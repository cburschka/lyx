#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "mathed/support.h"
#include "mathed/math_parser.h"
#include "support/LOstream.h"

using std::ostream;


MathDotsInset::MathDotsInset(latexkeys const * key)
	: key_(key)
{}


MathInset * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}     


void MathDotsInset::draw(Painter & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, key_->id);
	if (key_->id == LM_vdots || key_->id == LM_ddots)
		++x;
	if (key_->id != LM_vdots)
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, key_->id);
}


void MathDotsInset::metrics(MathStyles st) const
{
	size(st);
	mathed_char_dim(LM_TC_VAR, size(), 'M', ascent_, descent_, width_);
	switch (key_->id) {
		case LM_ldots: dh_ = 0; break;
		case LM_cdots: dh_ = ascent_ / 2; break;
		case LM_vdots: width_ /= 2;
		case LM_ddots: dh_ = ascent_; break;
	}
} 


void MathDotsInset::write(ostream & os, bool /* fragile */) const
{
	os << '\\' << key_->name << ' ';
}


void MathDotsInset::writeNormal(ostream & os) const
{
	os << "[" << key_->name << "] ";
}
