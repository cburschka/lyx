/**
 * \file InsetMathSize.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSize.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "MathParser.h"
#include "MathStream.h"

#include "support/convert.h"
#include "support/std_ostream.h"

using std::auto_ptr;


InsetMathSize::InsetMathSize(latexkeys const * l)
	: InsetMathNest(1), key_(l), style_(Styles(convert<int>(l->extra)))
{}


auto_ptr<InsetBase> InsetMathSize::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathSize(*this));
}


void InsetMathSize::metrics(MetricsInfo & mi, Dimension & dim) const
{
	StyleChanger dummy(mi.base, style_);
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	dim_ = dim;
}


void InsetMathSize::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, style_);
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathSize::write(WriteStream & os) const
{
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
}


void InsetMathSize::normalize(NormalStream & os) const
{
	os << '[' << key_->name << ' ' << cell(0) << ']';
}


void InsetMathSize::infoize(std::ostream & os) const
{
	os << "Size: " << key_->name;
}
