
#include <config.h>

#include "math_metricsinfo.h"
#include "math_support.h"
#include "debug.h"
#include "frontends/Painter.h"



MathMetricsBase::MathMetricsBase()
	: font(), style(LM_ST_TEXT), fontname("mathnormal"),
	  restrictwidth(false), textwidth(0)
{}




MathMetricsInfo::MathMetricsInfo()
	: fullredraw(false)
{}




MathPainterInfo::MathPainterInfo(Painter & p)
	: pain(p)
{}


void MathPainterInfo::draw(int x, int y, char c)
{
	pain.text(x, y, c, base.font);
}


MathStyles smallerScriptStyle(MathStyles st)
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

MathScriptChanger::MathScriptChanger(MathMetricsBase & mb)
	: MathStyleChanger(mb, smallerScriptStyle(mb.style))
{}



MathStyles smallerFracStyle(MathStyles st)
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

MathFracChanger::MathFracChanger(MathMetricsBase & mb)
	: MathStyleChanger(mb, smallerFracStyle(mb.style))
{}



MathShapeChanger::MathShapeChanger(LyXFont & font, LyXFont::FONT_SHAPE shape)
	: MathChanger<LyXFont, LyXFont::FONT_SHAPE>(font)
{
	save_ = orig_.shape();
	orig_.setShape(shape);
}

MathShapeChanger::~MathShapeChanger()
{
	orig_.setShape(save_);
}



MathStyleChanger::MathStyleChanger(MathMetricsBase & mb, MathStyles style)
	:	MathChanger<MathMetricsBase>(mb)
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

MathStyleChanger::~MathStyleChanger()
{
	orig_ = save_;
}



MathFontSetChanger::MathFontSetChanger(MathMetricsBase & mb, char const * name)
	:	MathChanger<MathMetricsBase>(mb)
{
	save_ = mb;	
	mb.fontname = name;
	augmentFont(mb.font, name);
}

MathFontSetChanger::~MathFontSetChanger()
{
	orig_ = save_;
}


MathWidthChanger::MathWidthChanger(MathMetricsBase & mb, int w)
	:	MathChanger<MathMetricsBase>(mb)
{
	save_ = mb;	
	mb.restrictwidth = true;
	mb.textwidth     = w;
}


MathWidthChanger::~MathWidthChanger()
{
	orig_ = save_;
}

