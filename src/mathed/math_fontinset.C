#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fontinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "support/LOstream.h"
#include "textpainter.h"



MathFontInset::MathFontInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{}


MathInset * MathFontInset::clone() const
{
	return new MathFontInset(*this);
}


MathInset::mode_type MathFontInset::currentMode() const
{
	if (key_->extra == "mathmode")
		return MATH_MODE;
	if (key_->extra == "textmode")
		return TEXT_MODE;
	return UNDECIDED_MODE;
}


void MathFontInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, key_->name.c_str());
	dim_ = cell(0).metrics(mi);
	metricsMarkers();
}


void MathFontInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathFontInset::metricsT(TextMetricsInfo const & mi) const
{
	dim_ = cell(0).metricsT(mi);
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
