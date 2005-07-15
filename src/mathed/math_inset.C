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

#include "math_inset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "debug.h"

#include "support/lstrings.h"

using std::string;
using std::ostream;
using std::endl;


MathArray & MathInset::cell(idx_type)
{
	static MathArray dummyCell;
	lyxerr << "I don't have a cell 1" << endl;
	return dummyCell;
}


MathArray const & MathInset::cell(idx_type) const
{
	static MathArray dummyCell;
	lyxerr << "I don't have a cell 2" << endl;
	return dummyCell;
}


void MathInset::dump() const
{
	lyxerr << "---------------------------------------------" << endl;
	WriteStream wi(lyxerr, false, true);
	write(wi);
	lyxerr << "\n---------------------------------------------" << endl;
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
	string const s = name();
	os << '\\' << s.c_str();S
	// We need an extra ' ' unless this is a single-char-non-ASCII name
	// or anything non-ASCII follows
	if (s.size() != 1 || isalpha(s[0]))
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


string const & MathInset::getType() const
{
	static string const t = "none";
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
