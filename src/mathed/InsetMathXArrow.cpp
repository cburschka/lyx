/**
 * \file InsetMathXArrow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathXArrow.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "LaTeXFeatures.h"

using namespace std;

namespace lyx {


InsetMathXArrow::InsetMathXArrow(docstring const & name)
	: InsetMathFracBase(), name_(name)
{}


Inset * InsetMathXArrow::clone() const
{
	return new InsetMathXArrow(*this);
}


void InsetMathXArrow::metrics(MetricsInfo & mi, Dimension & dim) const
{
	ScriptChanger dummy(mi.base);
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = max(dim0.width(), dim1.width()) + 10;
	dim.asc = dim0.height() + 10;
	dim.des = dim1.height();
	metricsMarkers(dim);
}


void InsetMathXArrow::draw(PainterInfo & pi, int x, int y) const
{
	ScriptChanger dummy(pi.base);
	cell(0).draw(pi, x + 5, y - 10);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	cell(1).draw(pi, x + 5, y + dim1.height());
	Dimension const dim = dimension(*pi.base.bv);
	mathed_draw_deco(pi, x + 1, y - 7, dim.wid - 2, 5, name_);
	drawMarkers(pi, x, y);
}


void InsetMathXArrow::write(WriteStream & os) const
{
	bool brace = os.pendingBrace();
	os.pendingBrace(false);
	if (os.latex() && os.textMode()) {
		os << "\\ensuremath{";
		os.textMode(false);
		brace = true;
	}

	os << '\\' << name_;
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(0) << '}';

	os.pendingBrace(brace);
}


void InsetMathXArrow::normalize(NormalStream & os) const
{
	os << "[xarrow " << name_ << ' ' <<  cell(0) << ' ' << cell(1) << ']';
}


void InsetMathXArrow::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
