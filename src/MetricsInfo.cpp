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
#include "LyXRC.h"
#include "MetricsInfo.h"

#include "insets/Inset.h"

#include "mathed/MathSupport.h"

#include "frontends/Painter.h"

#include "support/docstring.h"
#include "support/lassert.h"
#include "support/RefChanger.h"

using namespace std;


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// MetricsBase
//
/////////////////////////////////////////////////////////////////////////

MetricsBase::MetricsBase(BufferView * b, FontInfo f, int w)
	: bv(b), font(move(f)), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(w), solid_line_thickness_(1), solid_line_offset_(1),
	  dotted_line_thickness_(1)
{
	if (lyxrc.zoom >= 200) {
		// derive the line thickness from zoom factor
		// the zoom is given in percent
		// (increase thickness at 250%, 450% etc.)
		solid_line_thickness_ = (lyxrc.zoom + 50) / 200;
		// adjust line_offset_ too
		solid_line_offset_ = 1 + solid_line_thickness_ / 2;
	}
	if (lyxrc.zoom >= 100) {
		// derive the line thickness from zoom factor
		// the zoom is given in percent
		// (increase thickness at 150%, 250% etc.)
		dotted_line_thickness_ = (lyxrc.zoom + 50) / 100;
	}
}


Changer MetricsBase::changeFontSet(string const & name, bool cond)
{
	RefChanger<MetricsBase> rc = make_save(*this);
	if (!cond)
		rc->keep();
	else {
		ColorCode oldcolor = font.color();
		string const oldname = fontname;
		fontname = name;
		font = sane_font;
		augmentFont(font, name);
		font.setSize(rc->old.font.size());
		if (name != "lyxtex"
		    && ((isTextFont(oldname) && oldcolor != Color_foreground)
		        || (isMathFont(oldname) && oldcolor != Color_math)))
			font.setColor(oldcolor);
	}
	return move(rc);
}


/////////////////////////////////////////////////////////////////////////
//
// MetricsInfo
//
/////////////////////////////////////////////////////////////////////////

MetricsInfo::MetricsInfo(BufferView * bv, FontInfo font, int textwidth,
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
	do_spellcheck(true), full_repaint(true), background_color(Color_background)
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


Changer MetricsBase::changeScript(bool cond)
{
	switch (style) {
	case LM_ST_DISPLAY:
	case LM_ST_TEXT:
		return changeStyle(LM_ST_SCRIPT, cond);
	case LM_ST_SCRIPT:
	case LM_ST_SCRIPTSCRIPT:
		return changeStyle(LM_ST_SCRIPTSCRIPT, cond);
	}
	//remove Warning
	LASSERT(false, return Changer());
}


Changer MetricsBase::changeFrac(bool cond)
{
	switch (style) {
	case LM_ST_DISPLAY:
		return changeStyle(LM_ST_TEXT, cond);
	case LM_ST_TEXT:
		return changeStyle(LM_ST_SCRIPT, cond);
	case LM_ST_SCRIPT:
	case LM_ST_SCRIPTSCRIPT:
		return changeStyle(LM_ST_SCRIPTSCRIPT, cond);
	}
	//remove Warning
	return Changer();
}


Changer MetricsBase::changeStyle(Styles new_style, bool cond)
{
	static const int diff[4][4] =
		{ { 0, 0, -3, -5 },
		  { 0, 0, -3, -5 },
		  { 3, 3,  0, -2 },
		  { 5, 5,  2,  0 } };
	int t = diff[style][new_style];
	RefChanger<MetricsBase> rc = make_save(*this);
	if (!cond)
		rc->keep();
	else {
		if (t > 0)
			while (t--)
				font.incSize();
		else
			while (t++)
				font.decSize();
		style = new_style;
	}
	return move(rc);
}


} // namespace lyx
