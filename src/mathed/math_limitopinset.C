#include "math_limitopinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"


MathLimitOpInset::MathLimitOpInset(MathScriptInset const & scripts,
		MathArray const & core)
	: int_(new MathSymbolInset("int")),
		scripts_(scripts), core_(core)
{}


MathInset * MathLimitOpInset::clone() const
{
	return new MathLimitOpInset(*this);
}


void MathLimitOpInset::write(WriteStream & os) const
{
	scripts_.write2(int_.nucleus(), os);
	os << core_ << "d" << diff_;
}


void MathLimitOpInset::normalize(NormalStream &) const
{
	//os << "[int " << scripts_ << ' ' << core_ << ' ' << diff_ << ']'
}


void MathLimitOpInset::metrics(MathMetricsInfo const &) const
{
	lyxerr << "should not happen\n";
}


void MathLimitOpInset::draw(Painter &, int, int) const
{
	lyxerr << "should not happen\n";
}


void MathLimitOpInset::maplize(MapleStream &) const
{
	//os << name_ << '(' << cell(0) << ')';
}

void MathLimitOpInset::mathmlize(MathMLStream &) const
{
	//os << name_ << '(' << cell(0) << ')';
}
