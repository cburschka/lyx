#include "math_liminset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"


MathLimInset::MathLimInset
	(MathArray const & f, MathArray const & x, MathArray const & x0)
	: MathNestInset(3)
{
	cell(0) = f;
	cell(1) = x;
	cell(2) = x0;
}


MathInset * MathLimInset::clone() const
{
	return new MathLimInset(*this);
}


void MathLimInset::normalize(NormalStream & os) const
{
	os << "[lim " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void MathLimInset::metrics(MathMetricsInfo &) const
{
	lyxerr << "should not happen\n";
}


void MathLimInset::draw(MathPainterInfo &, int, int) const
{
	lyxerr << "should not happen\n";
}


void MathLimInset::maplize(MapleStream & os) const
{
	os << "limit(" << cell(0) << ',' << cell(1) << '=' << cell(2) << ')';
}


void MathLimInset::mathematicize(MathematicaStream & os) const
{
	os << "Lim[" << cell(0) << ',' << cell(1) << ',' << cell(2) << ']';
}


void MathLimInset::mathmlize(MathMLStream & os) const
{
	os << "lim(" << cell(0) << ',' << cell(1) << ',' << cell(2) << ')';
}


void MathLimInset::write(WriteStream &) const
{
	lyxerr << "should not happen\n";
}
