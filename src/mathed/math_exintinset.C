/**
 * \file math_exintinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_exintinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_symbolinset.h"
#include "debug.h"

#include <boost/scoped_ptr.hpp>


using std::string;
using std::auto_ptr;
using std::endl;


MathExIntInset::MathExIntInset(string const & name)
	: MathNestInset(4), symbol_(name)
{}

// 0 - core
// 1 - diff
// 2 - lower
// 3 - upper


auto_ptr<InsetBase> MathExIntInset::clone() const
{
	return auto_ptr<InsetBase>(new MathExIntInset(*this));
}


void MathExIntInset::symbol(string const & symbol)
{
	symbol_ = symbol;
}


bool MathExIntInset::hasScripts() const
{
	// take empty upper bound as "no scripts"
	return !cell(3).empty();
}



void MathExIntInset::normalize(NormalStream & os) const
{
	os << '[' << symbol_ << ' ' << cell(0) << ' ' << cell(1) << ' '
	   << cell(2) << ' ' << cell(3) << ']';
}


void MathExIntInset::metrics(MetricsInfo &, Dimension &) const
{
	lyxerr << "should not happen" << endl;
}


void MathExIntInset::draw(PainterInfo &, int, int) const
{
	lyxerr << "should not happen" << endl;
}


void MathExIntInset::maple(MapleStream & os) const
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


void MathExIntInset::maxima(MaximaStream & os) const
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

void MathExIntInset::mathematica(MathematicaStream & os) const
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


void MathExIntInset::mathmlize(MathMLStream & os) const
{
	boost::scoped_ptr<MathSymbolInset> sym(new MathSymbolInset(symbol_));
	//if (hasScripts())
	//	mathmlize(sym, os);
	//else
		sym->mathmlize(os);
	os << cell(0) << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << cell(1) << ETag("mrow");
}


void MathExIntInset::write(WriteStream &) const
{
	lyxerr << "should not happen" << endl;
}
