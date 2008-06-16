/**
 * \file InsetMathOverset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathOverset.h"
#include "MathData.h"
#include "MathStream.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"

using namespace std;

namespace lyx {

Inset * InsetMathOverset::clone() const
{
	return new InsetMathOverset(*this);
}


void InsetMathOverset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	FracChanger dummy(mi.base);
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	dim.wid = max(dim0.width(), dim1.wid) + 4;
	dim.asc = dim1.asc + dim0.height() + 4;
	dim.des = dim1.des;
	metricsMarkers(dim);
}


void InsetMathOverset::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int m  = x + dim.wid / 2;
	int yo = y - dim1.asc - dim0.des - 1;
	cell(1).draw(pi, m - dim1.wid / 2, y);
	FracChanger dummy(pi.base);
	cell(0).draw(pi, m - dim0.width() / 2, yo);
	drawMarkers(pi, x, y);
}


bool InsetMathOverset::idxFirst(Cursor & cur) const
{
	cur.idx() = 1;
	cur.pos() = 0;
	return true;
}


bool InsetMathOverset::idxLast(Cursor & cur) const
{
	cur.idx() = 1;
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathOverset::write(WriteStream & os) const
{
	bool brace = os.pendingBrace();
	os.pendingBrace(false);
	if (os.latex() && os.textMode()) {
		os << "\\ensuremath{";
		os.textMode(false);
	}
	os << "\\overset{" << cell(0) << "}{" << cell(1) << '}';
	os.pendingBrace(brace);
}


void InsetMathOverset::normalize(NormalStream & os) const
{
	os << "[overset " << cell(0) << ' ' << cell(1) << ']';
}


void InsetMathOverset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
