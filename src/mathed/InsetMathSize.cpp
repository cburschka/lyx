/**
 * \file InsetMathSize.cpp
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
#include "MathParser.h"
#include "MathStream.h"

#include "support/convert.h"

#include <ostream>


namespace lyx {

InsetMathSize::InsetMathSize(latexkeys const * l)
	: InsetMathNest(1), key_(l), style_(Styles(convert<int>(l->extra)))
{}


Inset * InsetMathSize::clone() const
{
	return new InsetMathSize(*this);
}


void InsetMathSize::metrics(MetricsInfo & mi, Dimension & dim) const
{
	StyleChanger dummy(mi.base, style_);
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathSize::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, style_);
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void InsetMathSize::write(WriteStream & os) const
{
	bool brace = ensureMath(os);
	os << "{\\" << key_->name << ' ' << cell(0) << '}';
	os.pendingBrace(brace);
}


void InsetMathSize::normalize(NormalStream & os) const
{
	os << '[' << key_->name << ' ' << cell(0) << ']';
}


void InsetMathSize::infoize(odocstream & os) const
{
	os << "Size: " << key_->name;
}


} // namespace lyx
