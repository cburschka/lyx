/**
 * \file math_boxedinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_boxedinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_streamstr.h"
#include "LaTeXFeatures.h"
#include "LColor.h"

#include "support/std_ostream.h"
#include "frontends/Painter.h"

using std::auto_ptr;


MathBoxedInset::MathBoxedInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathBoxedInset::clone() const
{
	return auto_ptr<InsetBase>(new MathBoxedInset(*this));
}


void MathBoxedInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers2(dim, 3); // 1 pixel space, 1 frame, 1 space
	dim_ = dim;
}


void MathBoxedInset::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, LColor::foreground);
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void MathBoxedInset::write(WriteStream & os) const
{
	os << "\\boxed{" << cell(0) << '}';
}


void MathBoxedInset::normalize(NormalStream & os) const
{
	os << "[boxed " << cell(0) << ']';
}


void MathBoxedInset::infoize(std::ostream & os) const
{
	os << "Boxed: ";
}


void MathBoxedInset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
}

