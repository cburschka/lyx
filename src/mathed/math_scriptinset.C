#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"
#include "support/LOstream.h"


MathScriptInset::MathScriptInset()
	: MathInset(2), up_(false), down_(false), limits_(0), symbol_(0)
{}


MathScriptInset::MathScriptInset(bool up, bool down, MathInset * symbol)
	: MathInset(2), up_(up), down_(down), limits_(0), symbol_(symbol)
{}


MathScriptInset::MathScriptInset(MathScriptInset const & p)
	: MathInset(p), up_(p.up_), down_(p.down_),
		limits_(p.limits_), symbol_(p.symbol_ ? p.symbol_->clone() : 0)
{}


MathScriptInset::~MathScriptInset()
{
	delete symbol_;
}


MathInset * MathScriptInset::clone() const
{   
	return new MathScriptInset(*this);
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


void MathScriptInset::Write(std::ostream & os, bool fragile) const
{
	if (symbol_) {
		symbol_->Write(os, fragile);
		if (limits())
			os << (limits() == 1 ? "\\limits" : "\\nolimits");
	}
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
	os << " ";
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


int MathScriptInset::limits() const
{  
	return limits_;
}


void MathScriptInset::limits(int limits) 
{  
	limits_ = limits;
}


bool MathScriptInset::hasLimits() const
{
	return
		symbol_ && (limits_ == 1 || (limits_ == 0 && size() == LM_ST_DISPLAY));
}


void MathScriptInset::WriteNormal(std::ostream & os) const
{
	if (limits() && symbol_) 
		os << "[" << (limits() ? "limits" : "nolimits") << "]";
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


void MathScriptInset::Metrics(MathStyles st)
{
	size_ = st;
	MathStyles tt = smallerStyleScript(st);
	
	xcell(0).Metrics(tt);
	xcell(1).Metrics(tt);

	width_   = std::max(xcell(0).width(), xcell(1).width());

	if (hasLimits()) {
		symbol_->Metrics(st);
		int wid  = symbol_->width();
		ascent_  = symbol_->ascent();
		descent_ = symbol_->descent();
		width_   = std::max(width_, wid);
		if (up()) {
			ascent_  += xcell(0).height() + 2;
			dy0_     = - (ascent_ - xcell(0).ascent());
		}
		if (down()) {
			descent_ += xcell(1).height() + 2;
			dy1_     = descent_ - xcell(1).descent();
		}
		dxx_   = (width_ - wid) / 2;
		dx0_   = (width_ - xcell(0).width()) / 2;
		dx1_   = (width_ - xcell(1).width()) / 2;
	} else {
		int asc;
		int des;
		int wid = 0;
		mathed_char_height(LM_TC_VAR, st, 'I', asc, des);
		if (symbol_) {
			symbol_->Metrics(st);
			wid  = symbol_->width();
			asc  = symbol_->ascent();
			des = symbol_->descent();
		}
		ascent_  = up()   ? xcell(0).height() + asc : 0;
		descent_ = down() ? xcell(1).height() + des : 0;
		width_  += wid;
		dy0_     = - asc - xcell(0).descent();
		dy1_     =   des + xcell(1).ascent();
		dx0_     = wid;
		dx1_     = wid;
		dxx_     = 0;
	}
}


void MathScriptInset::draw(Painter & pain, int x, int y)
{  
	xo(x);
	yo(y);

	if (symbol_)
		symbol_->draw(pain, x + dxx_, y);
	if (up())
		xcell(0).draw(pain, x + dx0_, y + dy0_);
	if (down())
		xcell(1).draw(pain, x + dx1_, y + dy1_);
}
