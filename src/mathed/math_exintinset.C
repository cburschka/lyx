#include "math_exintinset.h"
#include "math_support.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"


MathExIntInset::MathExIntInset()
	: int_(new MathSymbolInset("int"))
{}


MathInset * MathExIntInset::clone() const
{
	return new MathExIntInset(*this);
}


void MathExIntInset::differential(MathArray const & ar)
{
	diff_ = ar;
}


void MathExIntInset::core(MathArray const & ar)
{
	core_ = ar;
}


void MathExIntInset::scripts(MathAtom const & at)
{
	scripts_ = at;
}


void MathExIntInset::symbol(MathAtom const & at)
{
	int_ = at;
}


bool MathExIntInset::hasScripts() const
{
	return scripts_.hasNucleus();
}



void MathExIntInset::normalize(NormalStream & os) const
{
	os << "[int ";
	if (hasScripts())
		os << scripts_.nucleus();
	else 
		os << "{}";
	os << ' ' << core_ << ' ' << diff_ << ']';
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
	os << int_.nucleus() << '(' << core_ << ',' << diff_;
	if (hasScripts()) {
		MathScriptInset * p = scripts_->asScriptInset();
		os << '=' << p->down().data_ << ".." << p->up().data_;
	}
	os << ')';
}


void MathExIntInset::mathmlize(MathMLStream & os) const
{
	if (hasScripts())
		scripts_->asScriptInset()->mathmlize(int_.nucleus(), os);
	else 
		int_->mathmlize(os);
	os << core_ << "<mo> &InvisibleTimes; </mo>"
	   << MTag("mrow") << "<mo> &DifferentialD; </mo>"
	   << diff_ << ETag("mrow");
}


void MathExIntInset::write(WriteStream & os) const
{
	if (hasScripts())
		os << scripts_.nucleus();
	os << core_ << "d" << diff_;
}

