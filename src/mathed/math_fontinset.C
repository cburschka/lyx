/**
 * \file math_fontinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_fontinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "support/std_ostream.h"

using std::string;
using std::auto_ptr;


MathFontInset::MathFontInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{}


auto_ptr<InsetBase> MathFontInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathFontInset(*this));
}


MathInset::mode_type MathFontInset::currentMode() const
{
	if (key_->extra == "mathmode")
		return MATH_MODE;
	if (key_->extra == "textmode")
		return TEXT_MODE;
	return UNDECIDED_MODE;
}


void MathFontInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, key_->name.c_str());
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	dim_ = dim;
}


void MathFontInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


void MathFontInset::metricsT(TextMetricsInfo const & mi, Dimension &) const
{
	cell(0).metricsT(mi, dim_);
}


void MathFontInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


string MathFontInset::name() const
{
	return key_->name;
}


void MathFontInset::validate(LaTeXFeatures & features) const
{
	MathNestInset::validate(features);
	// Make sure amssymb is put in preamble if Blackboard Bold or
	// Fraktur used:
	if (key_->name == "mathfrak" || key_->name == "mathbb")
		features.require("amssymb");
}


void MathFontInset::infoize(std::ostream & os) const
{
	os << "Font: " << key_->name;
}
