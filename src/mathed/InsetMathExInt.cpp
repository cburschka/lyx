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
#include "MathData.h"
#include "MathStream.h"
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
	if (cell(0).size())
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

	if (cell(0).size())
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

	if (cell(0).size())
		os << cell(0) << ',';
	else
		os << '1' << ',';
	if (hasScripts())
		os << '{' << cell(1) << ',' << cell(2) << ',' << cell(3) << "}]";
	else
		os << cell(1) << ']';
}


void InsetMathExInt::mathmlize(MathStream & os) const
{
	InsetMathSymbol sym(symbol_);
	bool const lower = !cell(2).empty();
	bool const upper = !cell(3).empty();
	if (lower && upper)
		os << MTag("msubsup");
	else if (lower)
		os << MTag("msub");
	else if (upper)
		os << MTag("msup");
	os << MTag("mrow");
	sym.mathmlize(os);
	os << ETag("mrow");
	if (lower)
		os << MTag("mrow") << cell(2) << ETag("mrow");
	if (upper)
		os << MTag("mrow") << cell(3) << ETag("mrow");
	if (lower && upper)
		os << ETag("msubsup");
	else if (lower)
		os << ETag("msub");
	else if (upper)
		os << ETag("msup");
	os << cell(0) << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << cell(1) << ETag("mrow");
}


void InsetMathExInt::write(WriteStream &) const
{
	LYXERR0("should not happen");
}


} // namespace lyx
