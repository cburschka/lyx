/**
 * \file math_diffinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_diffinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"

using std::auto_ptr;
using std::endl;


MathDiffInset::MathDiffInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathDiffInset::clone() const
{
	return auto_ptr<InsetBase>(new MathDiffInset(*this));
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


void MathDiffInset::metrics(MetricsInfo &, Dimension &) const
{
	lyxerr << "should not happen" << endl;
}


void MathDiffInset::draw(PainterInfo &, int, int) const
{
	lyxerr << "should not happen" << endl;
}


void MathDiffInset::maple(MapleStream & os) const
{
	os << "diff(";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx != 0)
			os << ',';
		os << cell(idx);
	}
	os << ')';
}


void MathDiffInset::mathematica(MathematicaStream & os) const
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
	lyxerr << "should not happen" << endl;
}
