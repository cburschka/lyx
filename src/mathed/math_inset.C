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


using std::ostream;
using std::vector;


int MathInset::height() const
{
	return ascent() + descent();
}


ostream & operator<<(ostream & os, MathInset const & inset)
{
	WriteStream wi(os, false);
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


MathInset::idx_type MathInset::index(row_type row, col_type col) const
{
	if (row != 0)
		lyxerr << "illegal row: " << row << "\n";
	if (col != 0)
		lyxerr << "illegal col: " << col << "\n";
	return 0;
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


bool MathInset::idxUpDown(idx_type &, bool) const
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


void MathInset::normalize(NormalStream & os) const
{
	os << "[unknown ";
	WriteStream wi(os.os(), false);
	write(wi);
	os << "] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	WriteStream wi(lyxerr, false);
	write(wi);
	lyxerr << "\n---------------------------------------------\n";
}


void MathInset::validate(LaTeXFeatures &) const
{}


vector<MathInset::idx_type>
	MathInset::idxBetween(idx_type from, idx_type to) const
{
	vector<idx_type> res;
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


void MathInset::metricsT(TextMetricsInfo const &) const
{
	lyxerr << "MathInset::metricsT(Text) called directly!\n";
}


void MathInset::drawT(TextPainter &, int, int) const
{
	lyxerr << "MathInset::drawT(Text) called directly!\n";
}



void MathInset::write(WriteStream &) const
{
	lyxerr << "MathInset::write() called directly!\n";
}


void MathInset::octavize(OctaveStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void MathInset::maplize(MapleStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void MathInset::mathmlize(MathMLStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}
