#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"
#include "support/LOstream.h"


MathScriptInset::MathScriptInset(bool up, bool down)
	: MathUpDownInset(up, down)
{}


MathInset * MathScriptInset::clone() const
{   
	return new MathScriptInset(*this);
}


void MathScriptInset::WriteNormal(std::ostream & os) const
{
	if (up()) {
		os << "[superscript ";
		cell(0).WriteNormal(os);
		os << "] ";
	}
	if (down()) {
		os << "[subscript ";
		cell(1).WriteNormal(os);
		os << "] ";
	}
}


void MathScriptInset::idxDelete(int & idx, bool & popit, bool & deleteit)
{
	if (idx == 0) 
		up(false);
	else
		down(false);
	popit = true;
	deleteit = !(up() || down());
}
