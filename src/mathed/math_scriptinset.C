#include <config.h>
#include "debug.h"
#include "support/LOstream.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"


MathScriptInset::MathScriptInset(bool up)
	: MathNestInset(1), up_(up)
{}


MathInset * MathScriptInset::clone() const
{
	return new MathScriptInset(*this);
}


MathScriptInset const * MathScriptInset::asScriptInset() const
{
	return this;
}

void MathScriptInset::write(std::ostream & os, bool fragile) const
{
	cell(0).write(os, fragile);
}


void MathScriptInset::metrics(MathStyles st) const
{
	size_    = st;
	xcell(0).metrics(st);	
	width_   = xcell(0).width();
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();
	//lyxerr << "MathScriptInset::metrics: w: " << width_ << " a: " << ascent_
	//	<< " d: " << descent_ << "\n";
}


void MathScriptInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	xcell(0).draw(pain, x, y);
}
