/**
 * \file InsetMathLim.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathLim.h"
#include "MathData.h"
#include "MathStream.h"
#include "debug.h"


namespace lyx {

using std::auto_ptr;
using std::endl;


InsetMathLim::InsetMathLim
	(MathArray const & f, MathArray const & x, MathArray const & x0)
	: InsetMathNest(3)
{
	cell(0) = f;
	cell(1) = x;
	cell(2) = x0;
}


auto_ptr<InsetBase> InsetMathLim::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathLim(*this));
}


void InsetMathLim::normalize(NormalStream & os) const
{
	os << "[lim " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void InsetMathLim::metrics(MetricsInfo &, Dimension &) const
{
	lyxerr << "should not happen" << endl;
}


void InsetMathLim::draw(PainterInfo &, int, int) const
{
	lyxerr << "should not happen" << endl;
}


void InsetMathLim::maple(MapleStream & os) const
{
	os << "limit(" << cell(0) << ',' << cell(1) << '=' << cell(2) << ')';
}


void InsetMathLim::maxima(MaximaStream & os) const
{
	os << "limit(" << cell(0) << ',' << cell(1) << ',' << cell(2) << ')';
}


void InsetMathLim::mathematica(MathematicaStream & os) const
{
	os << "Limit[" << cell(0) << ',' << cell(1) << "-> " << cell(2) << ']';
}


void InsetMathLim::mathmlize(MathStream & os) const
{
	os << "lim(" << cell(0) << ',' << cell(1) << ',' << cell(2) << ')';
}


void InsetMathLim::write(WriteStream &) const
{
	lyxerr << "should not happen" << endl;
}


} // namespace lyx
