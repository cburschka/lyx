#include <config.h>

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


void MathFontInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, key_->name.c_str());
	cell(0).metrics(mi, dim_);
	metricsMarkers(1);
	dim = dim_;
}


void MathFontInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, key_->name.c_str());
	cell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathFontInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
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
