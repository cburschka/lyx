#include <config.h>

#include "math_exintinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_symbolinset.h"
#include "debug.h"

#include <boost/smart_ptr.hpp>

using std::endl;


MathExIntInset::MathExIntInset(string const & name)
	: MathNestInset(4), symbol_(name)
{}

// 0 - core
// 1 - diff
// 2 - lower
// 3 - upper


MathInset * MathExIntInset::clone() const
{
	return new MathExIntInset(*this);
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


void MathExIntInset::metrics(MathMetricsInfo const &) const
{
	lyxerr << "should not happen" << endl;
}


void MathExIntInset::draw(Painter &, int, int) const
{
	lyxerr << "should not happen" << endl;
}


void MathExIntInset::maplize(MapleStream & os) const
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
