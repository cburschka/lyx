/**
 * \file MetricsInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"
#include "Color.h"
#include "MetricsInfo.h"

#include "insets/Inset.h"

#include "mathed/MathSupport.h"

#include "frontends/Painter.h"

#include "support/docstring.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {


MetricsBase::MetricsBase()
	: bv(0), font(), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(0)
{}


MetricsBase::MetricsBase(BufferView * b, FontInfo const & f, int w)
	: bv(b), font(f), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(w)
{}


MetricsInfo::MetricsInfo(BufferView * bv, FontInfo const & font, int textwidth, 
	MacroContext const & mc)
	: base(bv, font, textwidth), macrocontext(mc)
{}


PainterInfo::PainterInfo(BufferView * bv, lyx::frontend::Painter & painter)
	: pain(painter), ltr_pos(false), erased_(false), selected(false),
	full_repaint(true), background_color(Color_background)
{
	base.bv = bv;
}


void PainterInfo::draw(int x, int y, char_type c)
{
	pain.text(x, y, c, base.font);
}


void PainterInfo::draw(int x, int y, docstring const & str)
{
	pain.text(x, y, str, base.font);
}


ColorCode PainterInfo::backgroundColor(Inset const * inset, bool sel) const
{
	ColorCode const color_bg = inset->backgroundColor();

	if (selected && sel)
		// This inset is in a selection
		return Color_selection;
	else {
		if (color_bg != Color_none)
			// This inset has its own color
			return color_bg;
		else {
			if (background_color == Color_none)
				// This inset has no own color and does not inherit a color
				return Color_background;
			else
				// This inset has no own color, but inherits a color
				return background_color;
		}
	}
}


Styles smallerScriptStyle(Styles st)
{
	switch (st) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:
			return LM_ST_SCRIPT;
		case LM_ST_SCRIPT:
		case LM_ST_SCRIPTSCRIPT:
		default: // shut up compiler
			return LM_ST_SCRIPTSCRIPT;
	}
}

ScriptChanger::ScriptChanger(MetricsBase & mb)
	: StyleChanger(mb, smallerScriptStyle(mb.style))
{}



Styles smallerFracStyle(Styles st)
{
	switch (st) {
		case LM_ST_DISPLAY:
			return LM_ST_TEXT;
		case LM_ST_TEXT:
			return LM_ST_SCRIPT;
		case LM_ST_SCRIPT:
		case LM_ST_SCRIPTSCRIPT:
		default: // shut up compiler
			return LM_ST_SCRIPTSCRIPT;
	}
}


FracChanger::FracChanger(MetricsBase & mb)
	: StyleChanger(mb, smallerFracStyle(mb.style))
{}



ArrayChanger::ArrayChanger(MetricsBase & mb)
	: StyleChanger(mb, mb.style == LM_ST_DISPLAY ? LM_ST_TEXT : mb.style)
{}


ShapeChanger::ShapeChanger(FontInfo & font, FontShape shape)
	: Changer<FontInfo, FontShape>(font)
{
	save_ = orig_.shape();
	orig_.setShape(shape);
}


ShapeChanger::~ShapeChanger()
{
	orig_.setShape(save_);
}



StyleChanger::StyleChanger(MetricsBase & mb, Styles style)
	:	Changer<MetricsBase>(mb)
{
	static const int diff[4][4] =
		{ { 0, 0, -3, -5 },
		  { 0, 0, -3, -5 },
		  { 3, 3,  0, -2 },
		  { 5, 5,  2,  0 } };
	save_ = mb;
	int t = diff[mb.style][style];
	if (t > 0)
		while (t--)
			mb.font.incSize();
	else
		while (t++)
			mb.font.decSize();
	mb.style = style;
}


StyleChanger::~StyleChanger()
{
	orig_ = save_;
}



FontSetChanger::FontSetChanger(MetricsBase & mb, char const * name)
	:	Changer<MetricsBase>(mb)
{
	save_ = mb;
	FontSize oldsize = save_.font.size();
	mb.fontname = name;
	mb.font = sane_font;
	augmentFont(mb.font, from_ascii(name));
	mb.font.setSize(oldsize);
}


FontSetChanger::FontSetChanger(MetricsBase & mb, docstring const & name)
	:	Changer<MetricsBase>(mb)
{
	save_ = mb;
	FontSize oldsize = save_.font.size();
	mb.fontname = to_utf8(name);
	mb.font = sane_font;
	augmentFont(mb.font, name);
	mb.font.setSize(oldsize);
}


FontSetChanger::~FontSetChanger()
{
	orig_ = save_;
}


WidthChanger::WidthChanger(MetricsBase & mb, int w)
	:	Changer<MetricsBase>(mb)
{
	save_ = mb;
	mb.textwidth = w;
}


WidthChanger::~WidthChanger()
{
	orig_ = save_;
}




ColorChanger::ColorChanger(FontInfo & font, string const & color)
	: Changer<FontInfo, string>(font)
{
	save_ = lcolor.getFromLyXName(color);
	font.setColor(lcolor.getFromLyXName(color));
}


ColorChanger::~ColorChanger()
{
	orig_.setColor(lcolor.getFromLyXName(save_));
}


} // namespace lyx
