#include "math_exintinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"


MathExIntInset::MathExIntInset(string const & name)
	: symbol_(name)
{}


MathInset * MathExIntInset::clone() const
{
	return new MathExIntInset(*this);
}


void MathExIntInset::index(MathArray const & ar)
{
	index_ = ar;
}


void MathExIntInset::core(MathArray const & ar)
{
	core_ = ar;
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
	return scripts_.hasNucleus();
}



void MathExIntInset::normalize(NormalStream & os) const
{
	os << '[' << symbol_.c_str() << ' ';
	if (hasScripts())
		os << scripts_.nucleus();
	else 
		os << "{}";
	os << ' ' << core_ << ' ' << index_ << ']';
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
	if (core_.size())
		os << core_;
	else 
		os << '1';
	os << ',' << index_;
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
	os << core_ << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << index_ << ETag("mrow");
}


void MathExIntInset::write(WriteStream &) const
{
	lyxerr << "should not happen\n";
}

