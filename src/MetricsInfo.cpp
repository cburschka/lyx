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
#include "ColorSet.h"
#include "MetricsInfo.h"

#include "insets/Inset.h"

#include "mathed/MathSupport.h"

#include "frontends/Painter.h"

#include "support/docstring.h"
#include "support/lassert.h"

using namespace std;


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// MetricsBase
//
/////////////////////////////////////////////////////////////////////////

MetricsBase::MetricsBase()
	: bv(0), font(), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(0)
{}


MetricsBase::MetricsBase(BufferView * b, FontInfo const & f, int w)
	: bv(b), font(f), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(w)
{}


/////////////////////////////////////////////////////////////////////////
//
// MetricsInfo
//
/////////////////////////////////////////////////////////////////////////

MetricsInfo::MetricsInfo(BufferView * bv, FontInfo const & font, int textwidth, 
	MacroContext const & mc)
	: base(bv, font, textwidth), macrocontext(mc)
{}


/////////////////////////////////////////////////////////////////////////
//
// PainterInfo
//
/////////////////////////////////////////////////////////////////////////

PainterInfo::PainterInfo(BufferView * bv, lyx::frontend::Painter & painter)
	: pain(painter), ltr_pos(false), change_(), selected(false),
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
	ColorCode const color_bg = inset->backgroundColor(*this);

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


Color PainterInfo::textColor(Color const & color) const
{
	if (change_.changed()) 
		return change_.color();
	if (selected)
		return Color_selectiontext;
	return color;
}


/////////////////////////////////////////////////////////////////////////
//
// ScriptChanger
//
/////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////
//
// FracChanger
//
/////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////
//
// ArrayChanger
//
/////////////////////////////////////////////////////////////////////////

ArrayChanger::ArrayChanger(MetricsBase & mb)
	: StyleChanger(mb, mb.style == LM_ST_DISPLAY ? LM_ST_TEXT : mb.style)
{}


/////////////////////////////////////////////////////////////////////////
//
// ShapeChanger
//
/////////////////////////////////////////////////////////////////////////

ShapeChanger::ShapeChanger(FontInfo & font, FontShape shape)
	: Changer<FontInfo, FontShape>(font, font.shape())
{
	orig_.setShape(shape);
}


ShapeChanger::~ShapeChanger()
{
	orig_.setShape(save_);
}


/////////////////////////////////////////////////////////////////////////
//
// StyleChanger
//
/////////////////////////////////////////////////////////////////////////

StyleChanger::StyleChanger(MetricsBase & mb, Styles style)
	: Changer<MetricsBase>(mb)
{
	static const int diff[4][4] =
		{ { 0, 0, -3, -5 },
		  { 0, 0, -3, -5 },
		  { 3, 3,  0, -2 },
		  { 5, 5,  2,  0 } };
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


/////////////////////////////////////////////////////////////////////////
//
// FontSetChanger
//
/////////////////////////////////////////////////////////////////////////

FontSetChanger::FontSetChanger(MetricsBase & mb, char const * name,
				bool really_change_font)
	: Changer<MetricsBase>(mb), change_(really_change_font)
{
	if (change_) {
		FontSize oldsize = save_.font.size();
		ColorCode oldcolor = save_.font.color();
		docstring const oldname = from_ascii(save_.fontname);
		mb.fontname = name;
		mb.font = sane_font;
		augmentFont(mb.font, from_ascii(name));
		mb.font.setSize(oldsize);
		if (string(name) != "lyxtex"
		    && ((isTextFont(oldname) && oldcolor != Color_foreground)
			|| (isMathFont(oldname) && oldcolor != Color_math)))
			mb.font.setColor(oldcolor);
	}
}


FontSetChanger::FontSetChanger(MetricsBase & mb, docstring const & name,
				bool really_change_font)
	: Changer<MetricsBase>(mb), change_(really_change_font)
{
	if (change_) {
		FontSize oldsize = save_.font.size();
		ColorCode oldcolor = save_.font.color();
		docstring const oldname = from_ascii(save_.fontname);
		mb.fontname = to_utf8(name);
		mb.font = sane_font;
		augmentFont(mb.font, name);
		mb.font.setSize(oldsize);
		if (name != "lyxtex"
		    && ((isTextFont(oldname) && oldcolor != Color_foreground)
			|| (isMathFont(oldname) && oldcolor != Color_math)))
			mb.font.setColor(oldcolor);
	}
}


FontSetChanger::~FontSetChanger()
{
	if (change_)
		orig_ = save_;
}


/////////////////////////////////////////////////////////////////////////
//
// WidthChanger
//
/////////////////////////////////////////////////////////////////////////

WidthChanger::WidthChanger(MetricsBase & mb, int w)
	: Changer<MetricsBase>(mb)
{
	mb.textwidth = w;
}


WidthChanger::~WidthChanger()
{
	orig_ = save_;
}


/////////////////////////////////////////////////////////////////////////
//
// ColorChanger
//
/////////////////////////////////////////////////////////////////////////

ColorChanger::ColorChanger(FontInfo & font, ColorCode color,
			   bool really_change_color)
	: Changer<FontInfo, ColorCode>(font, font.color()), change_(really_change_color)
{
	if (change_) {
		font.setColor(color);
	}
}


ColorChanger::~ColorChanger()
{
	if (change_)
		orig_.setColor(save_);
}


} // namespace lyx
