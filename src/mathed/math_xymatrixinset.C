/**
 * \file math_xymatrixinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_xymatrixinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

#include "LaTeXFeatures.h"
#include "support/std_ostream.h"


MathXYMatrixInset::MathXYMatrixInset()
	: MathGridInset(1, 1)
{}


std::auto_ptr<InsetBase> MathXYMatrixInset::doClone() const
{
	return std::auto_ptr<InsetBase>(new MathXYMatrixInset(*this));
}


int MathXYMatrixInset::colsep() const
{
	return 40;
}


int MathXYMatrixInset::rowsep() const
{
	return 40;
}


void MathXYMatrixInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY)
		mi.base.style = LM_ST_TEXT;
	MathGridInset::metrics(mi, dim);
}


void MathXYMatrixInset::write(WriteStream & os) const
{
	os << "\\xymatrix{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathXYMatrixInset::infoize(std::ostream & os) const
{
	os << "xymatrix ";
	MathGridInset::infoize(os);
}


void MathXYMatrixInset::normalize(NormalStream & os) const
{
	os << "[xymatrix ";
	MathGridInset::normalize(os);
	os << ']';
}


void MathXYMatrixInset::maple(MapleStream & os) const
{
	os << "xymatrix(";
	MathGridInset::maple(os);
	os << ')';
}
