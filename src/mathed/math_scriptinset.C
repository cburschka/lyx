#include <config.h>
#include "debug.h"
#include "support.h"
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

#if 0

bool MathScriptInset::idxUp(int & idx, int & pos) const
{
	if (idx == 0 || !up()) 
		return false;
	idx = 0;
	pos = 0;
	return true;
}

bool MathScriptInset::idxDown(int & idx, int & pos) const
{
	if (idx == 1 || !down()) 
		return false;
	idx = 1;
	pos = 0;
	return true;
}

bool MathScriptInset::idxFirst(int & idx, int & pos) const
{
	idx = up() ? 0 : 1;
	pos = 0;
	return true;
}

bool MathScriptInset::idxLast(int & idx, int & pos) const
{
	idx = down() ? 1 : 0;
	pos = cell(idx).size();
	return true;
}


bool MathScriptInset::idxFirstUp(int & idx, int & pos) const
{
	if (!up()) 
		return false;
	idx = 0;
	pos = 0;
	return true;
}


bool MathScriptInset::idxFirstDown(int & idx, int & pos) const
{
	if (!down()) 
		return false;
	idx = 1;
	pos = 0;
	return true;
}


bool MathScriptInset::idxLastUp(int & idx, int & pos) const
{
	if (!up()) 
		return false;
	idx = 0;
	pos = cell(idx).size();
	return true;
}


bool MathScriptInset::idxLastDown(int & idx, int & pos) const
{
	if (!down()) 
		return false;
	idx = 1;
	pos = cell(idx).size();
	return true;
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

#endif


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
}


void MathScriptInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	xcell(0).draw(pain, x, y);
}
