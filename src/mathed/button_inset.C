
#include <config.h>

#include "button_inset.h"
#include "math_support.h"
#include "metricsinfo.h"
#include "frontends/Painter.h"

#include <algorithm>

using std::max;


ButtonInset::ButtonInset()
	: MathNestInset(2)
{}


void ButtonInset::metrics(MetricsInfo & mi) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	if (editing()) {
		MathNestInset::metrics(mi);
		dim_.wid = cell(0).width() + cell(1).width() + 4;
		dim_.asc = max(cell(0).ascent(), cell(1).ascent());
		dim_.des = max(cell(0).descent(), cell(1).descent());
	} else {
		mathed_string_dim(mi.base.font, screenLabel(), dim_);
		dim_.wid += 10;
	}
}


void ButtonInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	if (editing()) {
		cell(0).draw(pi, x, y);
		cell(1).draw(pi, x + cell(0).width() + 2, y);
		mathed_draw_framebox(pi, x, y, this);
	} else {
		pi.pain.buttonText(x + 2, y, screenLabel(), pi.base.font);
	}
}
