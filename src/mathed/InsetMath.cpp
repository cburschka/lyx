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
#include "MathRow.h"
#include "MathStream.h"

#include "MetricsInfo.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"


using namespace std;

namespace lyx {

HullType hullType(docstring const & name)
{
	if (name == "none")      return hullNone;
	if (name == "simple")    return hullSimple;
	if (name == "equation")  return hullEquation;
	if (name == "eqnarray")  return hullEqnArray;
	if (name == "align")     return hullAlign;
	if (name == "alignat")   return hullAlignAt;
	if (name == "xalignat")  return hullXAlignAt;
	if (name == "xxalignat") return hullXXAlignAt;
	if (name == "multline")  return hullMultline;
	if (name == "gather")    return hullGather;
	if (name == "flalign")   return hullFlAlign;
	if (name == "regexp")    return hullRegexp;
	lyxerr << "unknown hull type '" << to_utf8(name) << "'" << endl;
	return hullUnknown;
}


docstring hullName(HullType type)
{
	switch (type) {
	case hullNone:       return from_ascii("none");
	case hullSimple:     return from_ascii("simple");
	case hullEquation:   return from_ascii("equation");
	case hullEqnArray:   return from_ascii("eqnarray");
	case hullAlign:      return from_ascii("align");
	case hullAlignAt:    return from_ascii("alignat");
	case hullXAlignAt:   return from_ascii("xalignat");
	case hullXXAlignAt:  return from_ascii("xxalignat");
	case hullMultline:   return from_ascii("multline");
	case hullGather:     return from_ascii("gather");
	case hullFlAlign:    return from_ascii("flalign");
	case hullRegexp:     return from_ascii("regexp");
	case hullUnknown:
		lyxerr << "unknown hull type" << endl;
		break;
	}
	return from_ascii("none");
}


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


marker_type InsetMath::marker(BufferView const *) const
{
	return nargs() > 0 ? marker_type::MARKER : marker_type::NO_MARKER;
}


bool InsetMath::addToMathRow(MathRow & mrow, MetricsInfo & mi) const
{
	MathRow::Element e(mi, MathRow::INSET, mathClass());
	e.inset = this;
	e.marker = mi.base.macro_nesting ? marker_type::NO_MARKER : marker(mi.base.bv);
	mrow.push_back(e);
	return true;
}


/// write LaTeX and LyX code
void InsetMath::writeLimits(TeXMathStream & os) const
{
	if (limits() == LIMITS) {
		os << "\\limits";
		os.pendingSpace(true);
	} else if (limits() == NO_LIMITS) {
		os << "\\nolimits";
		os.pendingSpace(true);
	}
}


void InsetMath::dump() const
{
	lyxerr << "---------------------------------------------" << endl;
	odocstringstream os;
	otexrowstream ots(os);
	TeXMathStream wi(ots, false, true, TeXMathStream::wsDefault);
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


void InsetMath::write(TeXMathStream & os) const
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


void InsetMath::mathmlize(MathMLStream & ms) const
{
	ms << "<!-- " << from_utf8(insetName(lyxCode())) << " -->";
	ms << MTagInline("mi");
	NormalStream ns(ms.os());
	normalize(ns);
	ms << ETagInline("mi");
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
	TeXMathStream wi(ots, false, false, TeXMathStream::wsDefault);
	at->write(wi);
	return os << to_utf8(oss.str());
}


odocstream & operator<<(odocstream & os, MathAtom const & at)
{
	otexrowstream ots(os);
	TeXMathStream wi(ots, false, false, TeXMathStream::wsDefault);
	at->write(wi);
	return os;
}


} // namespace lyx
