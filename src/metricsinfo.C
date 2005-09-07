/**
 * \file metricsinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"
#include "LColor.h"
#include "metricsinfo.h"

#include "mathed/math_support.h"

#include "frontends/Painter.h"

#include <boost/assert.hpp>

using std::string;


MetricsBase::MetricsBase()
	: bv(0), font(), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(0)
{}


MetricsBase::MetricsBase(BufferView * b, LyXFont const & f, int w)
	: bv(b), font(f), style(LM_ST_TEXT), fontname("mathnormal"),
	  textwidth(w)
{}



MetricsInfo::MetricsInfo()
{}


MetricsInfo::MetricsInfo(BufferView * bv, LyXFont const & font, int textwidth)
	: base(bv, font, textwidth)
{}



PainterInfo::PainterInfo(BufferView * bv, Painter & painter)
	: pain(painter), ltr_pos(false), erased_(false)
{
	base.bv = bv;
}


void PainterInfo::draw(int x, int y, char c)
{
	pain.text(x, y, c, base.font);
}


void PainterInfo::draw(int x, int y, std::string const & str)
{
	pain.text(x, y, str, base.font);
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


ShapeChanger::ShapeChanger(LyXFont & font, LyXFont::FONT_SHAPE shape)
	: Changer<LyXFont, LyXFont::FONT_SHAPE>(font)
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
	LyXFont::FONT_SIZE oldsize = save_.font.size();
	mb.fontname = name;
	mb.font = LyXFont();
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




ColorChanger::ColorChanger(LyXFont & font, string const & color)
	: Changer<LyXFont, string>(font)
{
	save_ = lcolor.getFromLyXName(color);
	font.setColor(lcolor.getFromLyXName(color));
}


ColorChanger::~ColorChanger()
{
	orig_.setColor(lcolor.getFromLyXName(save_));
}
