#include <config.h>
#include "debug.h"
#include "support.h"
#include "support/LOstream.h"
#include "support/LAssert.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scriptinset.h"


MathScriptInset::MathScriptInset()
	: MathNestInset(2), limits_(0)
{
	script_[0] = false;
	script_[1] = false;
}


MathScriptInset::MathScriptInset(bool up)
	: MathNestInset(2), limits_(0)
{
	script_[0] = !up;
	script_[1] = up;
}



MathInset * MathScriptInset::clone() const
{
	return new MathScriptInset(*this);
}


MathScriptInset const * MathScriptInset::asScriptInset() const
{
	return this;
}


MathScriptInset * MathScriptInset::asScriptInset()
{
	return this;
}


MathXArray const & MathScriptInset::up() const
{
	return xcell(1);
}


MathXArray const & MathScriptInset::down() const
{
	return xcell(0);
}


MathXArray & MathScriptInset::up()
{
	return xcell(1);
}


MathXArray & MathScriptInset::down()
{
	return xcell(0);
}


void MathScriptInset::ensure(bool up)
{
	script_[up] = true;
}


int MathScriptInset::dy0(MathInset const * nuc) const
{
	int nd = ndes(nuc);
	if (!hasDown())
		return nd;
	int des = down().ascent();
	if (hasLimits(nuc))
		des += nd + 2;
	else 
		des = std::max(des, nd);
	return des;
}


int MathScriptInset::dy1(MathInset const * nuc) const
{
	int na = nasc(nuc);
	if (!hasUp())
		return na;
	int asc = up().descent();
	if (hasLimits(nuc))
		asc += na + 2;
	else 
		asc = std::max(asc, na);
	asc = std::max(asc, mathed_char_ascent(LM_TC_VAR, mi_, 'I'));
	return asc;
}


int MathScriptInset::dx0(MathInset const * nuc) const
{
	lyx::Assert(hasDown());
	return hasLimits(nuc) ? (width(nuc) - down().width()) / 2 : nwid(nuc);
}


int MathScriptInset::dx1(MathInset const * nuc) const
{
	lyx::Assert(hasUp());
	return hasLimits(nuc) ? (width(nuc) - up().width()) / 2 : nwid(nuc);
}


int MathScriptInset::dxx(MathInset const * nuc) const
{
	//lyx::Assert(nuc());
	return hasLimits(nuc)  ?  (width(nuc) - nwid(nuc)) / 2  :  0;
}


int MathScriptInset::ascent(MathInset const * nuc) const
{
	return dy1(nuc) + (hasUp() ? up().ascent() : 0);
}


int MathScriptInset::descent(MathInset const * nuc) const
{
	return dy0(nuc) + (hasDown() ? down().descent() : 0);
}


int MathScriptInset::width(MathInset const * nuc) const
{
	int wid = 0;
	if (hasLimits(nuc)) {
		wid = nwid(nuc);
		if (hasUp())
			wid = std::max(wid, up().width());
		if (hasDown())
			wid = std::max(wid, down().width());
	} else {
		if (hasUp())
			wid = std::max(wid, up().width());
		if (hasDown())
			wid = std::max(wid, down().width());
		wid += nwid(nuc);
	}
	return wid;
}


int MathScriptInset::nwid(MathInset const * nuc) const
{
	return nuc ?
		nuc->width() :
		mathed_char_width(LM_TC_TEX, mi_, '.');
}


int MathScriptInset::nasc(MathInset const * nuc) const
{
	return nuc ? nuc->ascent()
		: mathed_char_ascent(LM_TC_VAR, mi_, 'I');
}


int MathScriptInset::ndes(MathInset const * nuc) const
{
	return nuc ? nuc->descent()
		: mathed_char_descent(LM_TC_VAR, mi_, 'I');
}


void MathScriptInset::metrics(MathMetricsInfo const & mi) const
{	
	metrics(0, mi);
}


void MathScriptInset::metrics(MathInset const * nuc,
	MathMetricsInfo const & mi) const
{	
	MathNestInset::metrics(mi);
	if (nuc)
		nuc->metrics(mi);

	ascent_  = ascent(nuc);
	descent_ = descent(nuc);
	width_   = width(nuc);
}


void MathScriptInset::draw(Painter & pain, int x, int y) const
{  
	//lyxerr << "unexpected call to MathScriptInset::draw()\n";
	draw(0, pain, x, y);
}


void MathScriptInset::draw(MathInset const * nuc, Painter & pain,
	int x, int y) const
{  
	xo(x);
	yo(y);

	if (nuc)
		nuc->draw(pain, x + dxx(nuc), y);
	else
		drawStr(pain, LM_TC_TEX, mi_, x + dxx(nuc), y, ".");

	if (hasUp())
		up().draw(pain, x + dx1(nuc), y - dy1(nuc));

	if (hasDown())
		down().draw(pain, x + dx0(nuc), y + dy0(nuc));
}


void MathScriptInset::write(MathWriteInfo & os) const
{  
	//lyxerr << "unexpected call to MathScriptInset::write()\n";
	write(0, os);
}


void MathScriptInset::write(MathInset const * nuc, MathWriteInfo & os) const
{
	if (nuc) {
		nuc->write(os);
		if (nuc->takesLimits()) {
			if (limits_ == -1)
				os << "\\nolimits ";
			if (limits_ == 1)
				os << "\\limits ";
		}
	}
	else
		os << "{}";

	if (hasDown() && down().data_.size())
		os << "_{" << down().data_ << '}';

	if (hasUp() && up().data_.size())
		os << "^{" << up().data_ << '}';
}


bool MathScriptInset::hasLimits(MathInset const * nuc) const
{
	return limits_ == 1 || (limits_ == 0 && nuc && nuc->isScriptable());
}


void MathScriptInset::removeEmptyScripts()
{
	for (int i = 0; i <= 1; ++i)
		if (script_[i] && !cell(i).size())
			script_[i] = false;
}


void MathScriptInset::removeScript(bool up)
{
	cell(up).clear();
	script_[up] = false;
}


bool MathScriptInset::has(bool up) const
{
	return script_[up];
}


bool MathScriptInset::hasUp() const
{
	return script_[1];
}


bool MathScriptInset::hasDown() const
{
	return script_[0];
}


bool MathScriptInset::idxRight(MathInset::idx_type &,
				 MathInset::pos_type &) const
{
	return false;
}


bool MathScriptInset::idxLeft(MathInset::idx_type &,
				MathInset::pos_type &) const
{
	return false;
}


