#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fboxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_parser.h"
#include "frontends/Painter.h"



MathFboxInset::MathFboxInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{}


MathInset * MathFboxInset::clone() const
{
	return new MathFboxInset(*this);
}


MathInset::mode_type MathFboxInset::currentMode() const
{
	if (key_->name == "fbox")
		return TEXT_MODE;
	return MATH_MODE;
}


void MathFboxInset::metrics(MathMetricsInfo & mi) const
{
	if (key_->name == "fbox") {
		MathFontSetChanger dummy(mi.base, "textnormal");
		dim_ = xcell(0).metrics(mi);
		metricsMarkers2(5); // 5 pixels margin
	} else {
		dim_ = xcell(0).metrics(mi);
		metricsMarkers2(5); // 5 pixels margin
	}
}


void MathFboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	pi.pain.rectangle(x + 1, y - ascent() + 1, width() - 2, height() - 2,
			LColor::black);
	if (key_->name == "fbox") {
		MathFontSetChanger dummy(pi.base, "textnormal");
		xcell(0).draw(pi, x + 5, y);
	} else {
		xcell(0).draw(pi, x + 5, y);
	}
}


void MathFboxInset::write(WriteStream & os) const
{
	os << '\\' << key_->name << '{' << cell(0) << '}';
}


void MathFboxInset::normalize(NormalStream & os) const
{
	os << '[' << key_->name << ' ' << cell(0) << ']';
}
