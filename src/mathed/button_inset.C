
#include "button_inset.h"
#include "math_support.h"
#include "frontends/Painter.h"


ButtonInset::ButtonInset()
	: MathNestInset(2)
{}


void ButtonInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	if (editing()) {
		MathNestInset::metrics(mi);
		width_   = xcell(0).width() + xcell(1).width() + 4;
		ascent_  = max(xcell(0).ascent(), xcell(1).ascent());
		descent_ = max(xcell(0).descent(), xcell(1).descent());
	} else {
		string s = screenLabel();
		mathed_string_dim(mi.base.font,
				 s, ascent_, descent_, width_);
		width_ += 10;
	}
}


void ButtonInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	if (editing()) {
		xcell(0).draw(pi, x, y);
		xcell(1).draw(pi, x + xcell(0).width() + 2, y);
		mathed_draw_framebox(pi, x, y, this);
	} else {
		pi.pain.buttonText(x + 2, y, screenLabel(),
			pi.base.font);
	}
}

