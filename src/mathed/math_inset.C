/**
 * \file math_inset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_mathmlstream.h"
#include "math_cursor.h"
#include "debug.h"

#include "formulabase.h"

using std::ostream;
using std::endl;


BufferView * MathInset::view() const
{
	return mathcursor ? mathcursor->formula()->view() : 0;
}


MathInset::size_type MathInset::nargs() const
{
	return 0;
}


MathArray dummyCell;

MathArray & MathInset::cell(idx_type)
{
	lyxerr << "I don't have a cell 1" << endl;
	return dummyCell;
}


MathArray const & MathInset::cell(idx_type) const
{
	lyxerr << "I don't have a cell 2" << endl;
	return dummyCell;
}


MathInset::idx_type MathInset::index(row_type row, col_type col) const
{
	if (row != 0)
		lyxerr << "illegal row: " << row << endl;
	if (col != 0)
		lyxerr << "illegal col: " << col << endl;
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


bool MathInset::idxUpDown2(idx_type &, pos_type &, bool, int) const
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
	lyxerr << "MathInset::getPos() called directly!" << endl;
	x = y = 0;
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------" << endl;
	WriteStream wi(lyxerr, false, true);
	write(wi);
	lyxerr << "\n---------------------------------------------" << endl;
}


bool MathInset::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	return from <= idx && idx <= to;
}


void MathInset::drawSelection(PainterInfo &,
	idx_type, pos_type, idx_type, pos_type) const
{
	lyxerr << "MathInset::drawSelection() called directly!" << endl;
}


void MathInset::metricsT(TextMetricsInfo const &, Dimension &) const
{
#ifdef WITH_WARNINGS
	lyxerr << "MathInset::metricsT(Text) called directly!" << endl;
#endif
}


void MathInset::drawT(TextPainter &, int, int) const
{
#ifdef WITH_WARNINGS
	lyxerr << "MathInset::drawT(Text) called directly!" << endl;
#endif
}



void MathInset::write(WriteStream & os) const
{
	os << '\\' << name().c_str();
	os.pendingSpace(true);
}


void MathInset::normalize(NormalStream & os) const
{
	os << '[' << name().c_str() << "] ";
}


void MathInset::octave(OctaveStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void MathInset::maple(MapleStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void MathInset::maxima(MaximaStream & os) const
{
	MapleStream ns(os.os());
	maple(ns);
}


void MathInset::mathematica(MathematicaStream & os) const
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


string const & MathInset::getType() const
{
	static string const t("none");
	return t;
}


string MathInset::name() const
{
	return "unknown";
}


ostream & operator<<(ostream & os, MathAtom const & at)
{
	WriteStream wi(os, false, false);
	at->write(wi);
	return os;
}


string MathInset::fileInsetLabel() const
{
	return "Formula";
}
