#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fontinset.h"
#include "debug.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "LaTeXFeatures.h"
#include "textpainter.h"
#include "frontends/Painter.h"



MathFontInset::MathFontInset(string const & name)
	: MathNestInset(1), name_(name)
{
	//lock(true);
}


MathInset * MathFontInset::clone() const
{
	return new MathFontInset(*this);
}


void MathFontInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, name_.c_str());
	dim_ = xcell(0).metrics(mi);
	metricsMarkers();
}


void MathFontInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "MathFontInset::draw\n";
	//MathNestInset::draw(pi, x, y);
	MathFontSetChanger dummy(pi.base, name_.c_str());
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
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void MathFontInset::normalize(NormalStream & os) const
{
	os << "[font_ " << name_ << " " << cell(0) << "]";
}


void MathFontInset::validate(LaTeXFeatures & features) const
{
	// Make sure amssymb is put in preamble if Blackboard Bold or
	// Fraktur used:
	if (name_ == "mathfrak" || name_ == "mathbb")
		features.require("amssymb");
}


void MathFontInset::infoize(std::ostream & os) const
{
	os << "Font: " << name_;
}
