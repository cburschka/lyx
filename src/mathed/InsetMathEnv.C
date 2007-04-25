/**
 * \file InsetMathEnv.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathEnv.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"
#include "support/std_ostream.h"


namespace lyx {


using std::string;
using std::auto_ptr;


InsetMathEnv::InsetMathEnv(docstring const & name)
	: InsetMathNest(1), name_(name)
{}


auto_ptr<InsetBase> InsetMathEnv::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathEnv(*this));
}


bool InsetMathEnv::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathEnv::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathEnv::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}' << cell(0) << "\\end{" << name_ << '}';
}


void InsetMathEnv::normalize(NormalStream & os) const
{
	os << "[env " << name_ << ' ' << cell(0) << ']';
}


void InsetMathEnv::infoize(odocstream & os) const
{
	os << "Env: " << name_;
}


} // namespace lyx
