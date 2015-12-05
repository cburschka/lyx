/**
 * \file InsetMathCancelto.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathCancelto.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"

#include "frontends/Painter.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetMathCancelto::InsetMathCancelto(Buffer * buf)
	: InsetMathNest(buf, 2)
{}


Inset * InsetMathCancelto::clone() const
{
	return new InsetMathCancelto(*this);
}


void InsetMathCancelto::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetMathNest::metrics(mi);
	Dimension const & dim0 = cell(0).dimension(*mi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*mi.base.bv);
	dim.asc = max(dim0.ascent() + 2, dim0.ascent() + dim1.ascent()) + 2 + 8;
	dim.des = max(dim0.descent() - 2, dim1.descent()) + 2;
	dim.wid = dim0.width() + dim1.width() + 10;
	metricsMarkers(dim);
}


void InsetMathCancelto::draw(PainterInfo & pi, int x, int y) const
{
	ColorCode const origcol = pi.base.font.color();

	// We first draw the text and then an arrow
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	cell(0).draw(pi, x + 1, y);
	cell(1).draw(pi, x + dim0.wid + 2 + 8, y - dim0.asc - 8);
		
	//Dimension const dim = dimension(*pi.base.bv);
	
	// y3____ ___
  	//          /|
	// y2_     / |
	//        /
	//       /
	//      /
	//     /
	// y1 /    | |
	//    x1  x2 x3

	int const x2 = x + dim0.wid;
	int const x3 = x2 + 8;
	int const x1 = x;
	int const y1 = y + dim0.des;
	int const y2 = y - dim0.asc;
	int const y3 = y2 - 8;

	// the main line
	pi.pain.line(x3, y3, x1, y1, origcol);
	// the arrow bars
	pi.pain.line(x3, y3, x2 + 2, y3, origcol);
	pi.pain.line(x3, y3, x3 - 2, y2 - 2, origcol);

	drawMarkers(pi, x, y);
}


void InsetMathCancelto::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\cancelto{" << cell(1) << "}{" << cell(0) << '}';
}


void InsetMathCancelto::normalize(NormalStream & os) const
{
	os << "[cancelto " << cell(1) << ' ' << cell(0) << ']';
}

bool InsetMathCancelto::idxUpDown(Cursor & cur, bool up) const
{
	Cursor::idx_type const target = up ? 1 : 0;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = up ? cur.lastpos() : 0;
	return true;
}

void InsetMathCancelto::infoize(odocstream & os) const
{
	os << "Cancelto";
}

void InsetMathCancelto::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	if (features.runparams().isLaTeX())
		features.require("cancel");
	InsetMathNest::validate(features);
}


} // namespace lyx
