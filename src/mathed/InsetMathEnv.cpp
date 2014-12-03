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

#include "support/gettext.h"
#include "support/lstrings.h"

#include <ostream>

using namespace lyx::support;

namespace lyx {

InsetMathEnv::InsetMathEnv(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 1), name_(name)
{}


Inset * InsetMathEnv::clone() const
{
	return new InsetMathEnv(*this);
}


void InsetMathEnv::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathEnv::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathEnv::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\begin{" << name_ << '}' << cell(0) << "\\end{" << name_ << '}';
}


void InsetMathEnv::normalize(NormalStream & os) const
{
	os << "[env " << name_ << ' ' << cell(0) << ']';
}


void InsetMathEnv::infoize(odocstream & os) const
{
	os << bformat(_("Environment: %1$s"), name_);
}


} // namespace lyx
