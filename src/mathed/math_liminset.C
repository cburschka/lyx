/**
 * \file math_liminset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_liminset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_symbolinset.h"
#include "debug.h"

using std::auto_ptr;
using std::endl;


MathLimInset::MathLimInset
	(MathArray const & f, MathArray const & x, MathArray const & x0)
	: MathNestInset(3)
{
	cell(0) = f;
	cell(1) = x;
	cell(2) = x0;
}


auto_ptr<InsetBase> MathLimInset::clone() const
{
	return auto_ptr<InsetBase>(new MathLimInset(*this));
}


void MathLimInset::normalize(NormalStream & os) const
{
	os << "[lim " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void MathLimInset::metrics(MetricsInfo &, Dimension &) const
{
	lyxerr << "should not happen" << endl;
}


void MathLimInset::draw(PainterInfo &, int, int) const
{
	lyxerr << "should not happen" << endl;
}


void MathLimInset::maple(MapleStream & os) const
{
	os << "limit(" << cell(0) << ',' << cell(1) << '=' << cell(2) << ')';
}


void MathLimInset::mathematica(MathematicaStream & os) const
{
	os << "Lim[" << cell(0) << ',' << cell(1) << ',' << cell(2) << ']';
}


void MathLimInset::mathmlize(MathMLStream & os) const
{
	os << "lim(" << cell(0) << ',' << cell(1) << ',' << cell(2) << ')';
}


void MathLimInset::write(WriteStream &) const
{
	lyxerr << "should not happen" << endl;
}
