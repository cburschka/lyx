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
#include "math_cursor.h"
#include "math_parser.h"
#include "debug.h"

#include "frontends/LyXView.h"
#include "frontends/Dialogs.h"
#include "BufferView.h"
#include "formulabase.h"


using std::ostream;
using std::vector;


int MathInset::height() const
{
	return ascent() + descent();
}


ostream & operator<<(ostream & os, MathAtom const & at)
{
	if (at.nucleus())
		os << *(at.nucleus());
	else
		os << "(nil)";
	return os;
}


ostream & operator<<(ostream & os, MathInset const & inset)
{
	WriteStream wi(os, false, false);
	inset.write(wi);
	return os;
}


MathInset::size_type MathInset::nargs() const
{
	return 0;
}


Dimension MathInset::dimensions() const
{
	lyxerr << "call MathInset::dimensions()\n";
	return Dimension(width(), ascent(), descent());
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
	return dummyCell.data();
}


MathArray const & MathInset::cell(idx_type) const
{
	lyxerr << "I don't have a cell 4\n";
	return dummyCell.data();
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


bool MathInset::idxUpDown(idx_type &, pos_type &, bool, int) const
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


void MathInset::getPos(idx_type, pos_type, int & x, int & y) const
{
	lyxerr << "MathInset::getPos() called directly!\n";
	x = y = 0;
}


void MathInset::normalize(NormalStream & os) const
{
	os << "[unknown ";
	WriteStream wi(os.os(), false, true);
	write(wi);
	os << "] ";
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	WriteStream wi(lyxerr, false, true);
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


void MathInset::metrics(MathMetricsInfo &) const
{
	lyxerr << "MathInset::metrics() called directly!\n";
}


void MathInset::draw(MathPainterInfo &, int, int) const
{
	lyxerr << "MathInset::draw() called directly!\n";
}


void MathInset::drawSelection(MathPainterInfo &,
	idx_type, pos_type, idx_type, pos_type) const
{
	lyxerr << "MathInset::drawSelection() called directly!\n";
}


void MathInset::metricsT(TextMetricsInfo const &) const
{
#ifdef WITH_WARNINGS
	lyxerr << "MathInset::metricsT(Text) called directly!\n";
#endif
}


void MathInset::drawT(TextPainter &, int, int) const
{
#ifdef WITH_WARNINGS
	lyxerr << "MathInset::drawT(Text) called directly!\n";
#endif
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


void MathInset::mathematicize(MathematicaStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void MathInset::mathmlize(MathMLStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


int MathInset::ascii(std::ostream &, int) const
{
	return 0;
}


int MathInset::linuxdoc(std::ostream &) const
{
	return 0;
}


int MathInset::docbook(std::ostream &, bool) const
{
	return 0;
}


int MathInset::dispatch(string const &, idx_type, pos_type) 
{
	return 0; // undispatched
}


std::vector<string> MathInset::getLabelList() const
{
	return std::vector<string>();
}


string const & MathInset::getType() const
{
	static string t("none");
	return t;
}


string MathInset::name() const
{
	return "unknown";
}


string asString(MathArray const & ar)
{
	std::ostringstream os;
	WriteStream ws(os);
	ws << ar;
	return os.str();
}


MathArray asArray(string const & str)
{
	MathArray ar;
	mathed_parse_cell(ar, str);
	return ar;
}


Dialogs * getDialogs()
{
	return mathcursor->formula()->view()->owner()->getDialogs();
}


LyXFunc * getLyXFunc()
{
	return mathcursor->formula()->view()->owner()->getLyXFunc();
}
