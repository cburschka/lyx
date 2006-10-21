/**
 * \file InsetMathXYMatrix.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathXYMatrix.h"
#include "MathMLStream.h"
#include "MathStream.h"

#include "LaTeXFeatures.h"
#include "support/std_ostream.h"


namespace lyx {


InsetMathXYMatrix::InsetMathXYMatrix()
	: InsetMathGrid(1, 1)
{}


std::auto_ptr<InsetBase> InsetMathXYMatrix::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetMathXYMatrix(*this));
}


int InsetMathXYMatrix::colsep() const
{
	return 40;
}


int InsetMathXYMatrix::rowsep() const
{
	return 40;
}


void InsetMathXYMatrix::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY)
		mi.base.style = LM_ST_TEXT;
	InsetMathGrid::metrics(mi, dim);
}


void InsetMathXYMatrix::write(WriteStream & os) const
{
	os << "\\xymatrix{";
	InsetMathGrid::write(os);
	os << "}\n";
}


void InsetMathXYMatrix::infoize(std::ostream & os) const
{
	os << "xymatrix ";
	InsetMathGrid::infoize(os);
}


void InsetMathXYMatrix::normalize(NormalStream & os) const
{
	os << "[xymatrix ";
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathXYMatrix::maple(MapleStream & os) const
{
	os << "xymatrix(";
	InsetMathGrid::maple(os);
	os << ')';
}


} // namespace lyx
