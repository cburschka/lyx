
#include <config.h>

#include "math_metricsinfo.h"
#include "math_support.h"
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




MathScriptChanger::MathScriptChanger(MathMetricsBase & mb)
	: MathChanger<MathMetricsBase>(mb)
{
	save_ = mb;
	switch (mb.style) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:
			mb.style = LM_ST_SCRIPT;
			mb.font.decSize();
			mb.font.decSize();
			break;
		case LM_ST_SCRIPT:
			mb.style = LM_ST_SCRIPTSCRIPT;
			mb.font.decSize();
		default:
			break;
	}
}

MathScriptChanger::~MathScriptChanger()
{
	orig_ = save_;
}




// decrease math size for fractions
MathFracChanger::MathFracChanger(MathMetricsBase & mb)
	: MathChanger<MathMetricsBase>(mb)
{
	save_ = mb;
	switch (mb.style) {
		case LM_ST_DISPLAY:
			mb.style = LM_ST_TEXT;
			break;
		case LM_ST_TEXT:
			mb.style = LM_ST_SCRIPT;
			mb.font.decSize();
			mb.font.decSize();
			break;
		case LM_ST_SCRIPT:
			mb.style = LM_ST_SCRIPTSCRIPT;
			mb.font.decSize();
			break;
		default:
			break;
	}
}

MathFracChanger::~MathFracChanger()
{
	orig_ = save_;
}




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




void changeSize(LyXFont & font, int diff)
{
	if (diff < 0) {
		font.decSize();
		changeSize(font, diff + 1);
	} else if (diff > 0) {
		font.incSize();
		changeSize(font, diff - 1);
	}
}

MathStyleChanger::MathStyleChanger(MathMetricsBase & mb, MathStyles style)
	:	MathChanger<MathMetricsBase>(mb)
{
	save_ = mb;
	changeSize(mb.font, mb.style - style);
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

