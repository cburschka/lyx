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


MathInset::MathInset()
	: size_(LM_ST_DISPLAY), xo_(0), yo_(0)
{}


int MathInset::height() const
{
	return ascent() + descent();
}


MathStyles MathInset::size() const
{
	return size_;
}


void MathInset::size(MathStyles s) const
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


void MathInset::xo(int x) const
{
	xo_ = x;
}


void MathInset::yo(int y) const
{
	yo_ = y;
}


int MathInset::nargs() const
{
	return 0;
}


MathXArray dummyCell;

MathXArray & MathInset::xcell(int)
{
	lyxerr << "I don't have a cell\n";
	return dummyCell;
}


MathXArray const & MathInset::xcell(int) const
{
	lyxerr << "I don't have a cell\n";
	return dummyCell;
}


MathArray & MathInset::cell(int)
{
	lyxerr << "I don't have a cell\n";
	return dummyCell.data_;
}


MathArray const & MathInset::cell(int) const
{
	lyxerr << "I don't have a cell\n";
	return dummyCell.data_;
}


void MathInset::substitute(MathArray & array, MathMacro const &) const
{
	array.push_back(clone());
}


bool MathInset::idxNext(int &, int &) const
{
	return false;
}


bool MathInset::idxRight(int &, int &) const
{
	return false;
}


bool MathInset::idxPrev(int &, int &) const
{
	return false;
}


bool MathInset::idxLeft(int &, int &) const
{
	return false;
}


bool MathInset::idxUp(int &, int &) const
{
	return false;
}


bool MathInset::idxDown(int &, int &) const
{
	return false;
}


bool MathInset::idxFirst(int &, int &) const
{
	return false;
}


bool MathInset::idxLast(int &, int &) const
{
	return false;
}


bool MathInset::idxHome(int &, int &) const
{
	return false;
}


bool MathInset::idxEnd(int &, int &) const
{
	return false;
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
	os << "[unknown] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	write(lyxerr, false);
	lyxerr << "\n---------------------------------------------\n";
}


void MathInset::push_back(unsigned char, MathTextCodes)
{
	lyxerr << "can't push without a cell\n";
}


void MathInset::push_back(MathInset *)
{
	lyxerr << "can't push without a cell\n";
}


bool MathInset::covers(int x, int y) const
{
	return
		x >= xo_ &&
		x <= xo_ + width() &&
		y >= yo_ - ascent() &&
		y <= yo_ + descent();
}


void MathInset::validate(LaTeXFeatures &) const
{}


std::vector<int> MathInset::idxBetween(int from, int to) const
{
	std::vector<int> res;
	for (int i = from; i <= to; ++i)
		res.push_back(i);
	return res;
}


void MathInset::metrics(MathStyles st) const
{
	lyxerr << "MathInset::metrics() called directly!\n";
	size_ = st;
}


void MathInset::draw(Painter &, int, int) const
{
	lyxerr << "MathInset::draw() called directly!\n";
}


void MathInset::write(std::ostream &, bool) const
{
	lyxerr << "MathInset::write() called directly!\n";
}
