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

#include "math_inset.h"
#include "debug.h"


int MathInset::workwidth;


MathInset::MathInset(int nargs, string const & name)
	: name_(name), width_(0), ascent_(0), descent_(0),
		size_(LM_ST_DISPLAY), code_(LM_TC_MIN), cells_(nargs), xo_(0), yo_(0)
{}


int MathInset::ascent() const
{
	return ascent_;
}


int MathInset::descent() const
{
	return descent_;
}


int MathInset::width() const
{
	return width_;
}


int MathInset::height() const
{
	return ascent_ + descent_;
}


string const & MathInset::name() const
{
	return name_;
}


void MathInset::setName(string const & n)
{
	name_ = n;
}


MathStyles MathInset::size() const
{
	return size_;
}


void MathInset::size(MathStyles s)
{
	size_ = s;
}


std::ostream & operator<<(std::ostream & os, MathInset const & inset)
{
	inset.write(os, false);
	return os;
}


int MathInset::xo() const
{
	return xo_;
}


int MathInset::yo() const
{
	return yo_;
}


void MathInset::xo(int x)
{
	xo_ = x;
}


void MathInset::yo(int y)
{
	yo_ = y;
}


int MathInset::nargs() const
{
	return cells_.size();
}


MathXArray & MathInset::xcell(int i)
{
	return cells_[i];
}


MathXArray const & MathInset::xcell(int i) const
{
	return cells_[i];
}


MathArray & MathInset::cell(int i)
{
	return cells_[i].data_;
}


MathArray const & MathInset::cell(int i) const
{
	return cells_[i].data_;
}


void MathInset::substitute(MathArray & array, MathMacro const & m) const
{
	MathInset * p = clone();
	for (int i = 0; i < nargs(); ++i)
		p->cell(i).substitute(m);
	array.push_back(p);
}


void MathInset::metrics(MathStyles st)
{
	size_ = st;
	for (int i = 0; i < nargs(); ++i)
		xcell(i).metrics(st);
}


void MathInset::draw(Painter & pain, int x, int y)
{
	xo_ = x;
	yo_ = y;
	for (int i = 0; i < nargs(); ++i)
		xcell(i).draw(pain, x + xcell(i).xo(), y + xcell(i).yo());
}


bool MathInset::idxNext(int & idx, int & pos) const
{
	if (idx + 1 >= nargs())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathInset::idxRight(int & idx, int & pos) const
{
	return idxNext(idx, pos);
}


bool MathInset::idxPrev(int & idx, int & pos) const
{
	if (idx == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}


bool MathInset::idxLeft(int & idx, int & pos) const
{
	return idxPrev(idx, pos);
}


bool MathInset::idxUp(int &, int &) const
{
	return false;
}


bool MathInset::idxDown(int &, int &) const
{
	return false;
}


bool MathInset::idxFirst(int & i, int & pos) const
{
	if (nargs() == 0)
		return false;
	i = 0;
	pos = 0;
	return true;
}


bool MathInset::idxLast(int & i, int & pos) const
{
	if (nargs() == 0)
		return false;
	i = nargs() - 1;
	pos = cell(i).size();
	return true;
}


bool MathInset::idxHome(int & /* idx */, int & pos) const
{
	if (pos == 0)
		return false;
	pos = 0;
	return true;
}


bool MathInset::idxEnd(int & idx, int & pos) const
{
	if (pos == cell(idx).size())
		return false;

	pos = cell(idx).size();
	return true;
}


bool MathInset::idxFirstUp(int &, int &) const
{
	return false;
}


bool MathInset::idxFirstDown(int &, int &) const
{
	return false;
}


void MathInset::idxDelete(int &, bool & popit, bool & deleteit)
{
	popit    = false;
	deleteit = false;
}


void MathInset::idxDeleteRange(int, int)
{}


bool MathInset::idxLastUp(int &, int &) const
{
	return false;
}


bool MathInset::idxLastDown(int &, int &) const
{
	return false;
}


void MathInset::getXY(int & x, int & y) const
{
   x = xo();
   y = yo();
}


/*
void MathInset::userSetSize(MathStyles sz)
{
	if (sz >= 0) {
		size_ = sz;      
		flag = flag & ~LMPF_FIXED_SIZE;
	}
}
*/

void MathInset::writeNormal(std::ostream & os) const
{
	os << "[" << name_ << "] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	write(lyxerr, false);
	lyxerr << "\n";
	for (int i = 0; i < nargs(); ++i)
		lyxerr << cell(i) << "\n";
	lyxerr << "---------------------------------------------\n";
}


void MathInset::push_back(unsigned char ch, MathTextCodes fcode)
{
	if (nargs())
		cells_.back().data_.push_back(ch, fcode);
	else
		lyxerr << "can't push without a cell\n";
}


void MathInset::push_back(MathInset * p)
{
	if (nargs())
		cells_.back().data_.push_back(p);
	else
		lyxerr << "can't push without a cell\n";
}


bool MathInset::covers(int x, int y) const
{
	return
		x >= xo_ &&
		x <= xo_ + width_ &&
		y >= yo_ - ascent_ &&
		y <= yo_ + descent_;
}


void MathInset::validate(LaTeXFeatures & features) const
{
	for (int i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}


std::vector<int> MathInset::idxBetween(int from, int to) const
{
	std::vector<int> res;
	for (int i = from; i <= to; ++i)
		res.push_back(i);
	return res;
}


MathTextCodes MathInset::code() const
{
	return code_;
}


void MathInset::code(MathTextCodes t)
{
	code_ = t;
}
