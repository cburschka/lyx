/**
 * \file math_casesinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_casesinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "LaTeXFeatures.h"
#include "support/std_ostream.h"

using std::auto_ptr;


MathCasesInset::MathCasesInset(row_type n)
	: MathGridInset(2, n, 'c', "ll")
{}


auto_ptr<InsetBase> MathCasesInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathCasesInset(*this));
}


void MathCasesInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	MathGridInset::metrics(mi);
	dim_.wid += 8;
	dim = dim_;
}


void MathCasesInset::draw(PainterInfo & pi, int x, int y) const
{
	mathed_draw_deco(pi, x + 1, y - dim_.ascent(), 6, dim_.height(), "{");
	MathGridInset::draw(pi, x + 8, y);
	setPosCache(pi, x, y);
}


void MathCasesInset::write(WriteStream & os) const
{
	if (os.fragile())
		os << "\\protect";
	os << "\\begin{cases}\n";
	MathGridInset::write(os);
	if (os.fragile())
		os << "\\protect";
	os << "\\end{cases}";
}


void MathCasesInset::normalize(NormalStream & os) const
{
	os << "[cases ";
	MathGridInset::normalize(os);
	os << ']';
}


void MathCasesInset::maple(MapleStream & os) const
{
	os << "cases(";
	MathGridInset::maple(os);
	os << ')';
}


void MathCasesInset::infoize(std::ostream & os) const
{
	os << "Cases ";
}


void MathCasesInset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	MathGridInset::validate(features);
}
