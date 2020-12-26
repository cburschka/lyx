/**
 * \file InsetMathExInt.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathExInt.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "InsetMathSymbol.h"

#include "support/debug.h"
#include "support/docstring.h"


namespace lyx {

InsetMathExInt::InsetMathExInt(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 4), symbol_(name)
{}

// 0 - core
// 1 - diff
// 2 - lower
// 3 - upper


Inset * InsetMathExInt::clone() const
{
	return new InsetMathExInt(*this);
}


void InsetMathExInt::symbol(docstring const & symbol)
{
	symbol_ = symbol;
}


bool InsetMathExInt::hasScripts() const
{
	// take empty upper bound as "no scripts"
	return !cell(3).empty();
}



void InsetMathExInt::normalize(NormalStream & os) const
{
	os << '[' << symbol_ << ' ' << cell(0) << ' ' << cell(1) << ' '
	   << cell(2) << ' ' << cell(3) << ']';
}


void InsetMathExInt::metrics(MetricsInfo &, Dimension &) const
{
	LYXERR0("should not happen");
}


void InsetMathExInt::draw(PainterInfo &, int, int) const
{
	LYXERR0("should not happen");
}


void InsetMathExInt::maple(MapleStream & os) const
{
	os << symbol_ << '(';
	if (!cell(0).empty())
		os << cell(0);
	else
		os << '1';
	os << ',' << cell(1);
	if (hasScripts())
		os << '=' << cell(2) << ".." << cell(3);
	os << ')';
}


void InsetMathExInt::maxima(MaximaStream & os) const
{
	if (symbol_ == "int")
		os << "integrate(";
	else
		os << symbol_ << '(';

	if (!cell(0).empty())
		os << cell(0) << ',';
	else
		os << '1' << ',';
	if (hasScripts())
		os << cell(1) << ',' << cell(2) << ',' << cell(3) << ')';
	else
		os << cell(1) << ')';
}

void InsetMathExInt::mathematica(MathematicaStream & os) const
{
	if (symbol_ == "int")
		os << "Integrate[";
	else if (symbol_ == "sum")
		os << "Sum[";
	else
		os << symbol_ << '[';

	if (!cell(0).empty())
		os << cell(0) << ',';
	else
		os << '1' << ',';
	if (hasScripts())
		os << '{' << cell(1) << ',' << cell(2) << ',' << cell(3) << "}]";
	else
		os << cell(1) << ']';
}


void InsetMathExInt::mathmlize(MathMLStream & ms) const
{
	// At the moment, we are not extracting sums and the like for MathML.
	// If we should decide to do so later, then we'll need to re-merge
	// r32566 and r32568.
	// So right now this only handles integrals.
	InsetMathSymbol sym(symbol_);
	bool const lower = !cell(2).empty();
	bool const upper = !cell(3).empty();
	if (lower && upper)
		ms << MTag("msubsup");
	else if (lower)
		ms << MTag("msub");
	else if (upper)
		ms << MTag("msup");
	ms << MTag("mrow");
	sym.mathmlize(ms);
	ms << ETag("mrow");
	if (lower)
		ms << MTag("mrow") << cell(2) << ETag("mrow");
	if (upper)
		ms << MTag("mrow") << cell(3) << ETag("mrow");
	if (lower && upper)
		ms << ETag("msubsup");
	else if (lower)
		ms << ETag("msub");
	else if (upper)
		ms << ETag("msup");
	ms << cell(0)
	   << "<" << from_ascii(ms.namespacedTag("mo")) << "> "
	   << " &InvisibleTimes; "
       << "</" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << MTag("mrow")
	   << "<" << from_ascii(ms.namespacedTag("mo")) << "> "
	   << " &DifferentialD; "
	   << "</" << from_ascii(ms.namespacedTag("mo")) << ">"
	   << cell(1)
	   << ETag("mrow");
}


void InsetMathExInt::htmlize(HtmlStream & os) const
{
	// At the moment, we are not extracting sums and the like for HTML.
	// So right now this only handles integrals.
	InsetMathSymbol sym(symbol_);
	bool const lower = !cell(2).empty();
	bool const upper = !cell(3).empty();

	os << MTag("span", "class='integral'")
	   << MTag("span", "class='intsym'");
	sym.htmlize(os, false);
	os << ETag("span");

	if (lower && upper) {
		os << MTag("span", "class='limits'")
		   << MTag("span") << cell(2) << ETag("span")
			 << MTag("span") << cell(3) << ETag("span")
			 << ETag("span");
	} else if (lower)
		os << MTag("sub", "class='limit'") << cell(2) << ETag("sub");
	else if (upper)
		os << MTag("sup", "class='limit'") << cell(3) << ETag("sup");
	os << cell(0) << "<b>d</b>" << cell(1) << ETag("span");
}


void InsetMathExInt::write(TeXMathStream &) const
{
	LYXERR0("should not happen");
}

} // namespace lyx
