
#include <config.h>

#include "metricsinfo.h"
#include "mathed/math_support.h"
#include "frontends/Painter.h"
#include "debug.h"



MetricsBase::MetricsBase()
	: font(), style(LM_ST_TEXT), fontname("mathnormal"),
	  restrictwidth(false), textwidth(0)
{}




MetricsInfo::MetricsInfo()
	: fullredraw(false)
{}




PainterInfo::PainterInfo(Painter & p)
	: pain(p)
{}


void PainterInfo::draw(int x, int y, char c)
{
	pain.text(x, y, c, base.font);
}


Styles smallerScriptStyle(Styles st)
{
	switch (st) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:
			return LM_ST_SCRIPT;
		case LM_ST_SCRIPT:
		case LM_ST_SCRIPTSCRIPT:
			return LM_ST_SCRIPTSCRIPT;
	}
	// shut up compiler
	lyxerr << "should not happen\n";
	return LM_ST_DISPLAY;
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
			return LM_ST_SCRIPTSCRIPT;
	}
	// shut up compiler
	lyxerr << "should not happen\n";
	return LM_ST_DISPLAY;
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
	static const int diff[4][4]  = { { 0, 0, -3, -5 },
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
	mb.fontname = name;
	augmentFont(mb.font, name);
}

FontSetChanger::~FontSetChanger()
{
	orig_ = save_;
}


WidthChanger::WidthChanger(MetricsBase & mb, int w)
	:	Changer<MetricsBase>(mb)
{
	save_ = mb;
	mb.restrictwidth = true;
	mb.textwidth     = w;
}


WidthChanger::~WidthChanger()
{
	orig_ = save_;
}
