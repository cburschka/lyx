/**
 * \file render_button.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "render_button.h"

#include "LColor.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

using std::string;
using std::auto_ptr;


RenderButton::RenderButton()
	: editable_(false)
{}


auto_ptr<RenderBase> RenderButton::clone() const
{
	return auto_ptr<RenderBase>(new RenderButton(*this));
}


void RenderButton::update(string const & text, bool editable)
{
	text_ = text;
	editable_ = editable;
}


void RenderButton::metrics(MetricsInfo &, Dimension & dim) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	if (editable_)
		font_metrics::buttonText(text_, font, dim.wid, dim.asc, dim.des);
	else
		font_metrics::rectText(text_, font, dim.wid, dim.asc, dim.des);

	dim.wid += 4;
}


void RenderButton::draw(PainterInfo & pi, int x, int y) const
{
	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command);
	font.decSize();

	if (editable_) {
		pi.pain.buttonText(x + 2, y, text_, font);
	} else {
		pi.pain.rectText(x + 2, y, text_, font,
				 LColor::commandbg, LColor::commandframe);
	}
}
