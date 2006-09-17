/**
 * \file InsetMathExInt.C
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
#include "MathMLStream.h"
#include "MathStream.h"
#include "InsetMathSymbol.h"
#include "debug.h"

#include <boost/scoped_ptr.hpp>


using std::string;
using std::auto_ptr;
using std::endl;


InsetMathExInt::InsetMathExInt(string const & name)
	: InsetMathNest(4), symbol_(name)
{}

// 0 - core
// 1 - diff
// 2 - lower
// 3 - upper


auto_ptr<InsetBase> InsetMathExInt::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathExInt(*this));
}


void InsetMathExInt::symbol(string const & symbol)
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
	lyxerr << "should not happen" << endl;
}


void InsetMathExInt::draw(PainterInfo &, int, int) const
{
	lyxerr << "should not happen" << endl;
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
	if ( symbol_ == "int" )
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
	if ( symbol_ == "int" )
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


void InsetMathExInt::mathmlize(MathMLStream & os) const
{
	boost::scoped_ptr<InsetMathSymbol> sym(new InsetMathSymbol(symbol_));
	//if (hasScripts())
	//	mathmlize(sym, os);
	//else
		sym->mathmlize(os);
	os << cell(0) << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << cell(1) << ETag("mrow");
}


void InsetMathExInt::write(WriteStream &) const
{
	lyxerr << "should not happen" << endl;
}
