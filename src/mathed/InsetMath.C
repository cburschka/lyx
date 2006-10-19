/**
 * \file InsetMath.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMath.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "debug.h"

#include "support/lstrings.h"

#include <boost/current_function.hpp>

using lyx::odocstream;

using std::string;
using std::endl;


MathArray & InsetMath::cell(idx_type)
{
	static MathArray dummyCell;
	lyxerr << BOOST_CURRENT_FUNCTION << ": I don't have any cell" << endl;
	return dummyCell;
}


MathArray const & InsetMath::cell(idx_type) const
{
	static MathArray dummyCell;
	lyxerr << BOOST_CURRENT_FUNCTION << ": I don't have any cell" << endl;
	return dummyCell;
}


void InsetMath::dump() const
{
	lyxerr << "---------------------------------------------" << endl;
	lyx::odocstringstream os;
	WriteStream wi(os, false, true);
	write(wi);
	lyxerr << lyx::to_utf8(os.str());
	lyxerr << "\n---------------------------------------------" << endl;
}


void InsetMath::metricsT(TextMetricsInfo const &, Dimension &) const
{
#ifdef WITH_WARNINGS
	lyxerr << "InsetMath::metricsT(Text) called directly!" << endl;
#endif
}


void InsetMath::drawT(TextPainter &, int, int) const
{
#ifdef WITH_WARNINGS
	lyxerr << "InsetMath::drawT(Text) called directly!" << endl;
#endif
}



void InsetMath::write(WriteStream & os) const
{
	string const s = name();
	os << '\\' << s.c_str();
	// We need an extra ' ' unless this is a single-char-non-ASCII name
	// or anything non-ASCII follows
	if (s.size() != 1 || isalpha(s[0]))
		os.pendingSpace(true);
}


void InsetMath::normalize(NormalStream & os) const
{
	os << '[' << name().c_str() << "] ";
}


void InsetMath::octave(OctaveStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void InsetMath::maple(MapleStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void InsetMath::maxima(MaximaStream & os) const
{
	MapleStream ns(os.os());
	maple(ns);
}


void InsetMath::mathematica(MathematicaStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


void InsetMath::mathmlize(MathMLStream & os) const
{
	NormalStream ns(os.os());
	normalize(ns);
}


HullType InsetMath::getType() const
{
	return hullNone;
}


string InsetMath::name() const
{
	return "unknown";
}


std::ostream & operator<<(std::ostream & os, MathAtom const & at)
{
	lyx::odocstringstream oss;
	WriteStream wi(oss, false, false);
	at->write(wi);
	return os << lyx::to_utf8(oss.str());
}


odocstream & operator<<(odocstream & os, MathAtom const & at)
{
	WriteStream wi(os, false, false);
	at->write(wi);
	return os;
}
