/**
 * \file math_envinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_envinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/LOstream.h"

using std::auto_ptr;


MathEnvInset::MathEnvInset(string const & name)
	: MathNestInset(1), name_(name)
{}


auto_ptr<InsetBase> MathEnvInset::clone() const
{
	return auto_ptr<InsetBase>(new MathEnvInset(*this));
}


void MathEnvInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim_);
	metricsMarkers();
	dim = dim_;
}


void MathEnvInset::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathEnvInset::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}' << cell(0) << "\\end{" << name_ << '}';
}


void MathEnvInset::normalize(NormalStream & os) const
{
	os << "[env " << name_ << ' ' << cell(0) << ']';
}


void MathEnvInset::infoize(std::ostream & os) const
{
	os << "Env: " << name_;
}
