/**
 * \file math_fboxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_fboxinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "math_streamstr.h"
#include "LColor.h"

#include "support/std_ostream.h"
#include "frontends/Painter.h"

using std::auto_ptr;


MathFboxInset::MathFboxInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathFboxInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathFboxInset(*this));
}


MathInset::mode_type MathFboxInset::currentMode() const
{
	return TEXT_MODE;
}


void MathFboxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim, 3); // 1 pixel space, 1 frame, 1 space
	dim_ = dim;
}


void MathFboxInset::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, LColor::foreground);
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void MathFboxInset::write(WriteStream & os) const
{
	os << "\\fbox{" << cell(0) << '}';
}


void MathFboxInset::normalize(NormalStream & os) const
{
	os << "[fbox " << cell(0) << ']';
}


void MathFboxInset::infoize(std::ostream & os) const
{
	os << "FBox: ";
}
