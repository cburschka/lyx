#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fontinset.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "textpainter.h"
#include "frontends/Painter.h"



MathFontInset::MathFontInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{
	//lock(true);
}


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
	dim_ = xcell(0).metrics(mi);
	metricsMarkers();
}


void MathFontInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "MathFontInset::draw\n";
	//MathNestInset::draw(pi, x, y);
	MathFontSetChanger dummy(pi.base, key_->name.c_str());
	xcell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathFontInset::metricsT(TextMetricsInfo const & mi) const
{
	dim_ = xcell(0).metricsT(mi);
}


void MathFontInset::drawT(TextPainter & pain, int x, int y) const
{
	//lyxerr << "drawing font code: " << code_ << '\n';
	xcell(0).drawT(pain, x, y);
}


void MathFontInset::write(WriteStream & os) const
{
	os << '\\' << key_->name << '{' << cell(0) << '}';
}


void MathFontInset::normalize(NormalStream & os) const
{
	os << "[font " << key_->name << " " << cell(0) << "]";
}


void MathFontInset::validate(LaTeXFeatures & features) const
{
	// Make sure amssymb is put in preamble if Blackboard Bold or
	// Fraktur used:
	if (key_->name == "mathfrak" || key_->name == "mathbb")
		features.require("amssymb");
}


void MathFontInset::infoize(std::ostream & os) const
{
	os << "Font: " << key_->name;
}
