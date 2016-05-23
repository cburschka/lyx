/**
 * \file RenderButton.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "RenderButton.h"

#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"


namespace lyx {


RenderButton::RenderButton()
	: editable_(false)
{}


RenderBase * RenderButton::clone(Inset const *) const
{
	return new RenderButton(*this);
}


void RenderButton::update(docstring const & text, bool editable)
{
	text_ = text;
	editable_ = editable;
}


void RenderButton::metrics(MetricsInfo &, Dimension & dim) const
{
	FontInfo font = sane_font;
	font.decSize();
	frontend::FontMetrics const & fm =
		theFontMetrics(font);

	if (editable_)
		fm.buttonText(text_, dim.wid, dim.asc, dim.des);
	else
		fm.rectText(text_, dim.wid, dim.asc, dim.des);

	dim_ = dim;
}


void RenderButton::draw(PainterInfo & pi, int x, int y) const
{
	// Draw it as a box with the LaTeX text
	FontInfo font = sane_font;
	font.setColor(Color_command);
	font.decSize();

	if (editable_) {
		pi.pain.buttonText(x, y, text_, font, renderState());
	} else {
		pi.pain.rectText(x, y, text_, font,
				 Color_commandbg, Color_commandframe);
	}
}


} // namespace lyx
