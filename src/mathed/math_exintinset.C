#include "math_exintinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"


MathExIntInset::MathExIntInset(string const & name)
	: MathNestInset(2), symbol_(name), scripts_(new MathScriptInset)
{}


MathInset * MathExIntInset::clone() const
{
	return new MathExIntInset(*this);
}


void MathExIntInset::scripts(MathAtom const & at)
{
	scripts_ = at;
}


MathAtom & MathExIntInset::scripts()
{
	return scripts_;
}


void MathExIntInset::symbol(string const & symbol)
{
	symbol_ = symbol;
}


bool MathExIntInset::hasScripts() const
{
	// take empty upper bound as "no scripts"
	return !scripts_->asScriptInset()->up().data_.empty();
}



void MathExIntInset::normalize(NormalStream & os) const
{
	os << '[' << symbol_.c_str() << ' ' << cell(0) << ' ' << cell(1);
	if (hasScripts())
		os << ' ' << scripts_.nucleus();
	os << ']';
}


void MathExIntInset::metrics(MathMetricsInfo const &) const
{
	lyxerr << "should not happen\n";
}


void MathExIntInset::draw(Painter &, int, int) const
{  
	lyxerr << "should not happen\n";
}


void MathExIntInset::maplize(MapleStream & os) const
{
	os << symbol_.c_str() << '(';
	if (cell(0).size())
		os << cell(0);
	else 
		os << '1';
	os << ',' << cell(1);
	if (hasScripts()) {
		MathScriptInset * p = scripts_->asScriptInset();
		os << '=' << p->down().data_ << ".." << p->up().data_;
	}
	os << ')';
}


void MathExIntInset::mathmlize(MathMLStream & os) const
{
	MathSymbolInset * sym = new MathSymbolInset(symbol_.c_str());
	if (hasScripts())
		scripts_->asScriptInset()->mathmlize(sym, os);
	else 
		sym->mathmlize(os);
	delete sym;
	os << cell(0) << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << cell(1) << ETag("mrow");
}


void MathExIntInset::write(WriteStream &) const
{
	lyxerr << "should not happen\n";
}

