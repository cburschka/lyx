#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


MathDotsInset::MathDotsInset(string const & name)
	: name_(name)
{}


MathInset * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}     


void MathDotsInset::draw(Painter & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
	char const c = name_[0];
	if (c == 'v' || c == 'd')
		++x;
	if (c != 'v')
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
}


void MathDotsInset::metrics(MathStyles st) const
{
	size(st);
	mathed_char_dim(LM_TC_VAR, size(), 'M', ascent_, descent_, width_);
	switch (name_[0]) {
		case 'l': dh_ = 0; break;
		case 'c': dh_ = ascent_ / 2; break;
		case 'v': width_ /= 2;
		case 'd': dh_ = ascent_; break;
	}
} 


void MathDotsInset::write(ostream & os, bool /* fragile */) const
{
	os << '\\' << name_ << ' ';
}


void MathDotsInset::writeNormal(ostream & os) const
{
	os << "[" << name_ << "] ";
}
