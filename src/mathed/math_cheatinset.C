#ifdef __GNUG__
#pragma implementation
#endif

#include "math_cheatinset.h"
#include "support.h"
#include "support/LOstream.h"



MathCheatInset::MathCheatInset(double w)
	: wid_(w)
{}


MathInset * MathCheatInset::clone() const
{
	return new MathCheatInset(*this);
}


void MathCheatInset::draw(Painter & pain, int x, int y) const
{}


void MathCheatInset::write(std::ostream &, bool) const
{}


void MathCheatInset::writeNormal(std::ostream &) const
{}


void MathCheatInset::metrics(MathStyles st) const
{
	// mimic -.1em
	mathed_char_dim(LM_TC_CONST, st, 'm', ascent_, descent_, width_);
	width_ *= wid_;
}
