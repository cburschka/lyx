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
	: editable_(false), inherit_font_(false)
{}


RenderBase * RenderButton::clone(Inset const *) const
{
	return new RenderButton(*this);
}


void RenderButton::update(docstring const & text, bool editable,
                          bool inherit)
{
	text_ = text;
	editable_ = editable;
	inherit_font_ = inherit;
}


void RenderButton::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontInfo font = inherit_font_ ? mi.base.font : sane_font;
	font.decSize();
	frontend::FontMetrics const & fm = theFontMetrics(font);

	fm.buttonText(text_, Inset::textOffset(mi.base.bv), dim.wid, dim.asc, dim.des);

	dim_ = dim;
}


void RenderButton::draw(PainterInfo & pi, int x, int y) const
{
	// Draw it as a box with the LaTeX text
	FontInfo font = inherit_font_ ? pi.base.font : sane_font;
	font.setColor(Color_command);
	font.decSize();

	if (editable_) {
		pi.pain.buttonText(x, y, text_, font,
		                   renderState() ? Color_buttonhoverbg : Color_buttonbg,
		                   Color_buttonframe, Inset::textOffset(pi.base.bv));
	} else {
		pi.pain.buttonText(x, y, text_, font,
		                   Color_commandbg, Color_commandframe,
		                   Inset::textOffset(pi.base.bv));
	}
}


} // namespace lyx
