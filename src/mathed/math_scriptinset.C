#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <functional>

#include "math_scriptinset.h"
#include "LColor.h"
#include "Painter.h"
#include "debug.h"
#include "mathed/support.h"
#include "support/LOstream.h"


MathScriptInset::MathScriptInset()
	: MathInset("script", LM_OT_SCRIPT, 2), up_(false), down_(false)
{}

MathScriptInset::MathScriptInset(bool up, bool down)
	: MathInset("script", LM_OT_SCRIPT, 2), up_(up), down_(down)
{}


MathInset * MathScriptInset::Clone() const
{   
	return new MathScriptInset(*this);
}


void MathScriptInset::Metrics(MathStyles st)
{
	MathInset::Metrics(st);
	size_    = st;
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 2; 
	if (up())
		ascent_  = std::max(ascent_, xcell(0).height() + 9);
	if (down())
		descent_ = std::max(descent_, xcell(1).height());
}


bool MathScriptInset::up() const
{
	return up_;
}

bool MathScriptInset::down() const
{
	return down_;
}

void MathScriptInset::up(bool b)
{
	up_ = b;
}

void MathScriptInset::down(bool b)
{
	down_ = b;
}


void MathScriptInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y);
	if (up())
		xcell(0).draw(pain, x, y - xcell(0).descent() - 9);
	if (down())
		xcell(1).draw(pain, x, y + xcell(1).ascent());
}


void MathScriptInset::Write(std::ostream & os, bool fragile) const
{
	if (up()) {
		os << "^{";
		cell(0).Write(os, fragile);
		os << "}";
	}
	if (down()) {
		os << "_{";
		cell(1).Write(os, fragile);
		os << "}";
	}
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

bool MathScriptInset::idxRight(int &, int &) const
{
	return false;
}

bool MathScriptInset::idxLeft(int &, int &) const
{
	return false;
}


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


bool MathScriptInset::idxDelete(int idx)
{
	if (idx == 0) {
		up(false);
		return !down();
	} else {
		down(false);
		return !up();
	}
}
