/**
 * \file math_substackinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_substackinset.h"
#include "math_mathmlstream.h"
#include "support/LOstream.h"

using std::auto_ptr;


MathSubstackInset::MathSubstackInset()
	: MathGridInset(1, 1)
{}


auto_ptr<InsetBase> MathSubstackInset::clone() const
{
	return auto_ptr<InsetBase>(new MathSubstackInset(*this));
}


void MathSubstackInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY) {
		StyleChanger dummy(mi.base, LM_ST_TEXT);
		MathGridInset::metrics(mi);
	} else {
		MathGridInset::metrics(mi);
	}
	metricsMarkers();
	dim = dim_;
}


void MathSubstackInset::draw(PainterInfo & pi, int x, int y) const
{
	MathGridInset::draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathSubstackInset::infoize(std::ostream & os) const
{
	os << "Substack ";
}


void MathSubstackInset::write(WriteStream & os) const
{
	os << "\\substack{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathSubstackInset::normalize(NormalStream & os) const
{
	os << "[substack ";
	MathGridInset::normalize(os);
	os << ']';
}


void MathSubstackInset::maple(MapleStream & os) const
{
	os << "substack(";
	MathGridInset::maple(os);
	os << ')';
}
