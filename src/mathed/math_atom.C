/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_atom.h"
#include "math_scriptinset.h"
#include "debug.h"
#include "support.h"
#include "support/LAssert.h"


MathAtom::MathAtom()
	: nucleus_(0), limits_(0), xo_(0), yo_(0)
{
	script_[0] = 0;
	script_[1] = 0;
}


MathAtom::MathAtom(MathInset * p)
	: nucleus_(p), limits_(0), xo_(0), yo_(0)
{
	script_[0] = 0;
	script_[1] = 0;
}



MathAtom::MathAtom(MathAtom const & p)
{
	copy(p);
}


void MathAtom::operator=(MathAtom const & p)
{
	if (this != &p) {
		done();
		copy(p);
	}
}


MathAtom::~MathAtom()
{
	done();
}


void MathAtom::done()
{
	delete nucleus_;
	delete script_[0];
	delete script_[1];
}


void MathAtom::copy(MathAtom const & p)
{
	//cerr << "calling MathAtom::copy\n";
	xo_        = p.xo_;
	yo_        = p.yo_;
	limits_    = p.limits_;
	nucleus_   = p.nucleus_;
	script_[0] = p.script_[0];
	script_[1] = p.script_[1];
	if (nucleus_)
		nucleus_ = nucleus_->clone();
	if (script_[0])
		script_[0]  = new MathScriptInset(*script_[0]);
	if (script_[1])
		script_[1]  = new MathScriptInset(*script_[1]);
}


int MathAtom::height() const
{
	return ascent() + descent();
}


std::ostream & operator<<(std::ostream & os, MathAtom const & atom)
{
	atom.write(os, false);
	return os;
}


int MathAtom::xo() const
{
	return xo_;
}


int MathAtom::yo() const
{
	return yo_;
}


void MathAtom::xo(int x) const
{
	xo_ = x;
}


void MathAtom::yo(int y) const
{
	yo_ = y;
}


void MathAtom::getXY(int & x, int & y) const
{
   x = xo();
   y = yo();
}


/*
void MathAtom::userSetSize(MathStyles sz)
{
	if (sz >= 0) {
		size_ = sz;      
		flag = flag & ~LMPF_FIXED_SIZE;
	}
}
*/

void MathAtom::writeNormal(std::ostream & os) const
{
	os << "[unknown] ";
}


void MathAtom::dump() const
{
	lyxerr << "---------------------------------------------\n";
	write(lyxerr, false);
	lyxerr << "\n---------------------------------------------\n";
}


bool MathAtom::covers(int x, int y) const
{
	return
		x >= xo_ &&
		x <= xo_ + width() &&
		y >= yo_ - ascent() &&
		y <= yo_ + descent();
}


MathScriptInset * MathAtom::ensure(bool up)
{
	if (!script_[up])
		script_[up] = new MathScriptInset(up);
	return script_[up];
}


void MathAtom::validate(LaTeXFeatures &) const
{}


void MathAtom::metrics(MathStyles st) const
{
	MathStyles tt = smallerStyleScript(st);
	if (nucleus())
		nucleus()->metrics(st);
	if (up())
		up()->metrics(tt);
	if (down())
		down()->metrics(tt);
}


MathScriptInset * MathAtom::up() const
{
	return script_[1];
}


MathScriptInset * MathAtom::down() const
{
	return script_[0];
}


int MathAtom::dy0() const
{
	lyx::Assert(down());
	int des = down()->ascent();
	if (hasLimits())
		des += ndes() + 2;
	else 
		des = std::max(des, ndes());
	return des;
}


int MathAtom::dy1() const
{
	lyx::Assert(up());
	int asc = up()->descent();
	if (hasLimits())
		asc += nasc() + 2;
	else 
		asc = std::max(asc, nasc());
	asc = std::max(asc, mathed_char_ascent(LM_TC_VAR, LM_ST_TEXT, 'I'));
	return asc;
}


int MathAtom::dx0() const
{
	lyx::Assert(down());
	return hasLimits() ? (width() - down()->width()) / 2 : nwid();
}


int MathAtom::dx1() const
{
	lyx::Assert(up());
	return hasLimits() ? (width() - up()->width()) / 2 : nwid();
}


int MathAtom::dxx() const
{
	lyx::Assert(nucleus());
	return hasLimits() ? (width() - nwid()) / 2 : 0;
}


int MathAtom::ascent() const
{
	int asc = nasc();
	if (up()) 
		asc += hasLimits() ? up()->height() + 2 : up()->ascent();
	return asc;
}


int MathAtom::descent() const
{
	int des = ndes();
	if (down()) 
		des += hasLimits() ? down()->height() + 2 : down()->descent();
	return des;
}


int MathAtom::width() const
{
	int wid = 0;
	if (hasLimits()) {
		wid = nwid();
		if (up())
			wid = std::max(wid, up()->width());
		if (down())
			wid = std::max(wid, down()->width());
	} else {
		if (up())
			wid = std::max(wid, up()->width());
		if (down())
			wid = std::max(wid, down()->width());
		wid += nwid();
	}
	return wid;
}


int MathAtom::nwid() const
{
	return nucleus() ? nucleus()->width() : 0;
}


int MathAtom::nasc() const
{
	return nucleus() ? nucleus()->ascent()
		: mathed_char_ascent(LM_TC_VAR, LM_ST_TEXT, 'I');
}


int MathAtom::ndes() const
{
	return nucleus() ? nucleus()->descent()
		: mathed_char_descent(LM_TC_VAR, LM_ST_TEXT, 'I');
}


void MathAtom::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);
	if (nucleus())
		nucleus()->draw(pain, x + dxx(), y);
	if (up())
		up()->draw(pain, x + dx1(), y - dy1());
	if (down())
		down()->draw(pain, x + dx0(), y + dy0());
}


void MathAtom::write(std::ostream & os, bool fragile) const
{
	if (nucleus()) {
		nucleus()->write(os, fragile);
		if (nucleus()->takesLimits()) {
			if (limits_ == -1)
				os << "\\nolimits ";
			if (limits_ == 1)
				os << "\\limits ";
		}
	} else
		os << "{}";

	if (up()) {
		os << "^{";
		up()->write(os, fragile);
		os << "}";
	}

	if (down()) {
		os << "_{";
		down()->write(os, fragile);
		os << "}";
	}
}


bool MathAtom::hasLimits() const
{
	return
		limits_ == 1 || (limits_ == 0 && nucleus() && nucleus()->isScriptable());
}


void MathAtom::substitute(MathArray & array, MathMacro const & m) const
{
	if (nucleus())
		nucleus()->substitute(array, m);
	if (up())
		up()->substitute(array, m);
	if (down())
		down()->substitute(array, m);
}


void MathAtom::removeEmptyScripts()
{
	for (int i = 0; i <= 1; ++i)
		if (script_[i] && !script_[i]->cell(0).size()) {
			delete script_[i];
			script_[i] = 0;
		}
}
