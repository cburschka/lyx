#include "math_exintinset.h"
#include "math_support.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"


MathExIntInset::MathExIntInset(MathScriptInset const & scripts,
		MathArray const & core, MathArray const & diff)
	: int_(new MathSymbolInset("int")),
		scripts_(scripts), core_(core), diff_(diff)
{}


MathInset * MathExIntInset::clone() const
{
	return new MathExIntInset(*this);
}


void MathExIntInset::write(WriteStream & os) const
{
	scripts_.write(int_.nucleus(), os);
	os << core_ << "d" << diff_;
}


void MathExIntInset::normalize(NormalStream & os) const
{
	//os << "[int " << scripts_ << ' ' << core_ << ' ' << diff_ << ']'
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
	//os << name_.c_str() << '(' << cell(0) << ')';
}

void MathExIntInset::mathmlize(MathMLStream & os) const
{
	//os << name_.c_str() << '(' << cell(0) << ')';
}
