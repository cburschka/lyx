#include "math_diffinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"


MathDiffInset::MathDiffInset()
	: MathNestInset(1)
{}


MathInset * MathDiffInset::clone() const
{
	return new MathDiffInset(*this);
}


void MathDiffInset::addDer(MathArray const & der)
{
	cells_.push_back(der);
}


void MathDiffInset::normalize(NormalStream & os) const
{
	os << "[diff";
	for (idx_type idx = 0; idx < nargs(); ++idx)
		os << ' ' << cell(idx);
	os << ']';
}


void MathDiffInset::metrics(MathMetricsInfo &) const
{
	lyxerr << "should not happen\n";
}


void MathDiffInset::draw(MathPainterInfo &, int, int) const
{
	lyxerr << "should not happen\n";
}


void MathDiffInset::maplize(MapleStream & os) const
{
	os << "diff(";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx != 0)
			os << ',';
		os << cell(idx);
	}
	os << ')';
}


void MathDiffInset::mathematicize(MathematicaStream & os) const
{
	os << "Dt[";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx != 0)
			os << ',';
		os << cell(idx);
	}
	os << ']';
}


void MathDiffInset::mathmlize(MathMLStream & os) const
{
	os << "diff(";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx != 0)
			os << ',';
		os << cell(idx);
	}
	os << ')';
}


void MathDiffInset::write(WriteStream &) const
{
	lyxerr << "should not happen\n";
}
