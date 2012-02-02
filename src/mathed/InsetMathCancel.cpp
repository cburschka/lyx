/**
 * \file InsetMathCancel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathCancel.h"

#include "LaTeXFeatures.h"
#include "MathStream.h"

#include "frontends/Painter.h"

#include <ostream>

namespace lyx {


InsetMathCancel::InsetMathCancel(Buffer * buf, Kind k)
	: InsetMathNest(buf, 1), kind_(k)
{}


Inset * InsetMathCancel::clone() const
{
	return new InsetMathCancel(*this);
}


void InsetMathCancel::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathCancel::draw(PainterInfo & pi, int x, int y) const
{
	// We first draw the text and then an arrow
	ColorCode const origcol = pi.base.font.color();
	cell(0).draw(pi, x + 1, y);
	Dimension const dim = dimension(*pi.base.bv);

	/*
	 * y1 \    /
	 *     \  /
	 *      \/
	 *      /\
	 *     /  \
	 * y2 /	   \
	 *    x1  x2
	 */

	int const x2 = x + dim.wid;
	int const x1 = x;
	int const y1 = y - dim.asc;
	int const y2 = y + dim.des;

	if (kind_ == cancel)
		pi.pain.line(x2, y1, x1, y2, origcol);
	else if (kind_ == bcancel)
		pi.pain.line(x2, y2, x1, y1, origcol);
	else if (kind_ == xcancel) {
		pi.pain.line(x2, y1, x1, y2, origcol);
		pi.pain.line(x2, y2, x1, y1, origcol);
	}

	drawMarkers(pi, x, y);
}


void InsetMathCancel::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	switch (kind_) {
	case cancel:
		os << "\\cancel{";
		break;
	case bcancel:
		os << "\\bcancel{";
		break;
	case xcancel:
		os << "\\xcancel{";
		break;
	}
	os << cell(0) << '}';
}


void InsetMathCancel::normalize(NormalStream & os) const
{
	switch (kind_) {
	case cancel:
		os << "[cancel ";
		break;
	case bcancel:
		os << "[bcancel ";
		break;
	case xcancel:
		os << "[xcancel ";
		break;
	}
	os << cell(0) << ']';
}


void InsetMathCancel::infoize(odocstream & os) const
{
	switch (kind_) {
	case cancel:
		os << "Cancel";
		break;
	case bcancel:
		os << "Bcancel";
		break;
	case xcancel:
		os << "Xcancel";
		break;
	}
}

void InsetMathCancel::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	if (features.runparams().isLaTeX())
		features.require("cancel");
	InsetMathNest::validate(features);
}

} // namespace lyx
