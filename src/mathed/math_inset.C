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

#include <config.h>

#include "Lsstream.h"
#include "math_inset.h"
#include "math_scriptinset.h"
#include "math_mathmlstream.h"
#include "debug.h"


MathInset::MathInset()
{}


MathInset::~MathInset()
{}


int MathInset::height() const
{
	return ascent() + descent();
}


std::ostream & operator<<(std::ostream & os, MathInset const & inset)
{
	WriteStream wi(0, os, false);
	inset.write(wi);
	return os;
}


MathInset::size_type MathInset::nargs() const
{
	return 0;
}


MathXArray dummyCell;

MathXArray & MathInset::xcell(idx_type)
{
	lyxerr << "I don't have a cell 1\n";
	return dummyCell;
}


MathXArray const & MathInset::xcell(idx_type) const
{
	lyxerr << "I don't have a cell 2\n";
	return dummyCell;
}


MathArray & MathInset::cell(idx_type)
{
	lyxerr << "I don't have a cell 3\n";
	return dummyCell.data_;
}


MathArray const & MathInset::cell(idx_type) const
{
	lyxerr << "I don't have a cell 4\n";
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


bool MathInset::idxFirstUp(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxFirstDown(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxLast(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxLastUp(idx_type &, pos_type &) const
{
	return false;
}


bool MathInset::idxLastDown(idx_type &, pos_type &) const
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


void MathInset::normalize(NormalStream & os) const
{
	os << "[unknown ";
	WriteStream wi(0, os.os_, false);
	write(wi);
	os << "] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	WriteStream wi(0, lyxerr, false);
	write(wi);
	lyxerr << "\n---------------------------------------------\n";
}


bool MathInset::covers(int, int) const
{
	lyxerr << "MathInset::covers() called directly!\n";
	return false;
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


void MathInset::metrics(MathMetricsInfo const &) const
{
	lyxerr << "MathInset::metrics() called directly!\n";
}


void MathInset::draw(Painter &, int, int) const
{
	lyxerr << "MathInset::draw() called directly!\n";
}


void MathInset::write(WriteStream &) const
{
	lyxerr << "MathInset::write() called directly!\n";
}


void MathInset::octavize(OctaveStream & os) const
{
	NormalStream ns(os.os_);
	normalize(ns);
}


void MathInset::maplize(MapleStream & os) const
{
	NormalStream ns(os.os_);
	normalize(ns);
}


void MathInset::mathmlize(MathMLStream & os) const
{
	NormalStream ns(os.os_);
	normalize(ns);
}
