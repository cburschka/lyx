/**
 * \file InsetMath.cpp
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
#include "MathStream.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

docstring InsetMath::name() const
{
	return from_utf8("Unknown");
}


MathData & InsetMath::cell(idx_type)
{
	static MathData dummyCell(&buffer());
	LYXERR0("I don't have any cell");
	return dummyCell;
}


MathData const & InsetMath::cell(idx_type) const
{
	static MathData dummyCell;
	LYXERR0("I don't have any cell");
	return dummyCell;
}


void InsetMath::dump() const
{
	lyxerr << "---------------------------------------------" << endl;
	odocstringstream os;
	otexrowstream ots(os);
	WriteStream wi(ots, false, true, WriteStream::wsDefault);
	write(wi);
	lyxerr << to_utf8(os.str());
	lyxerr << "\n---------------------------------------------" << endl;
}


void InsetMath::metricsT(TextMetricsInfo const &, Dimension &) const
{
	LYXERR0("InsetMath::metricsT(Text) called directly!");
}


void InsetMath::drawT(TextPainter &, int, int) const
{
	LYXERR0("InsetMath::drawT(Text) called directly!");
}


void InsetMath::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	docstring const s = name();
	os << "\\" << s;
	// We need an extra ' ' unless this is a single-char-non-ASCII name
	// or anything non-ASCII follows
	if (s.size() != 1 || isAlphaASCII(s[0]))
		os.pendingSpace(true);
}


int InsetMath::plaintext(odocstringstream &, 
        OutputParams const &, size_t) const
{
	// all math plain text output shall take place in InsetMathHull
	LATTEST(false);
	return 0;
}


void InsetMath::normalize(NormalStream & os) const
{
	os << '[' << name() << "] ";
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


void InsetMath::mathmlize(MathStream & os) const
{
	os << "<!-- " << from_utf8(insetName(lyxCode())) << " -->";
	os << MTag("mi");
	NormalStream ns(os.os());
	normalize(ns);
	os << ETag("mi");
}


void InsetMath::htmlize(HtmlStream & os) const
{
	os << "<!-- " << from_utf8(insetName(lyxCode())) << " -->";
	os << MTag("span", "style='color: red;'");
	NormalStream ns(os.os());
	normalize(ns);
	os << ETag("span");
}


HullType InsetMath::getType() const
{
	return hullNone;
}


ostream & operator<<(ostream & os, MathAtom const & at)
{
	odocstringstream oss;
	otexrowstream ots(oss);
	WriteStream wi(ots, false, false, WriteStream::wsDefault);
	at->write(wi);
	return os << to_utf8(oss.str());
}


odocstream & operator<<(odocstream & os, MathAtom const & at)
{
	otexrowstream ots(os);
	WriteStream wi(ots, false, false, WriteStream::wsDefault);
	at->write(wi);
	return os;
}


} // namespace lyx
