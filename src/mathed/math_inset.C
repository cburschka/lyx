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
#include "math_scriptinset.h"
#include "debug.h"


int MathInset::workwidth;


MathInset::MathInset()
	: xo_(0), yo_(0)
{}


MathInset::~MathInset()
{}


int MathInset::height() const
{
	return ascent() + descent();
}


MathStyles MathInset::size() const
{
	return size_.size;
}


std::ostream & operator<<(std::ostream & os, MathInset const & inset)
{
	MathWriteInfo wi(0, os, false);
	inset.write(wi);
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


MathInset::size_type MathInset::nargs() const
{
	return 0;
}


MathXArray dummyCell;

MathXArray & MathInset::xcell(idx_type)
{
	lyxerr << "I don't have a cell\n";
	return dummyCell;
}


MathXArray const & MathInset::xcell(idx_type) const
{
	lyxerr << "I don't have a cell\n";
	return dummyCell;
}


MathArray & MathInset::cell(idx_type)
{
	lyxerr << "I don't have a cell\n";
	return dummyCell.data_;
}


MathArray const & MathInset::cell(idx_type) const
{
	lyxerr << "I don't have a cell\n";
	return dummyCell.data_;
}


void MathInset::substitute(MathMacro const &)
{}


bool MathInset::idxNext(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxRight(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxPrev(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxLeft(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxUp(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxDown(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxFirst(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxLast(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxHome(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxEnd(idx_type &, pos_type &) const
{
	return false;
}


void MathInset::idxDelete(idx_type &, bool & popit, bool & deleteit)
{
	popit    = false;
	deleteit = false;
}


void MathInset::idxDeleteRange(idx_type, idx_type)
{}


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
	os << "[unknown ";
	MathWriteInfo wi(0, os, false);
	write(wi);
	os << "] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	MathWriteInfo wi(0, lyxerr, false);
	write(wi);
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
	//lyxerr << "cover? p: " << this << " x: " << x << " y: " <<  y
	//	<< " xo_: " << xo_ << " yo_: " << yo_  << endl;
	return
		x >= xo_ &&
		x <= xo_ + width() &&
		y >= yo_ - ascent() &&
		y <= yo_ + descent();
}


void MathInset::validate(LaTeXFeatures &) const
{}


std::vector<MathInset::idx_type>
	MathInset::idxBetween(idx_type from, idx_type to) const
{
	std::vector<idx_type> res;
	for (idx_type i = from; i <= to; ++i)
		res.push_back(i);
	return res;
}


void MathInset::metrics(MathMetricsInfo const & st) const
{
	lyxerr << "MathInset::metrics() called directly!\n";
	size_ = st;
}


void MathInset::draw(Painter &, int, int) const
{
	lyxerr << "MathInset::draw() called directly!\n";
}


void MathInset::write(MathWriteInfo &) const
{
	lyxerr << "MathInset::write() called directly!\n";
}
