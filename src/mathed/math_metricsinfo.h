#ifndef MATH_METRICSINFO_H
#define MATH_METRICSINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "LString.h"

class Painter;


/// Standard Math Sizes (Math mode styles)
enum MathStyles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


struct MathMetricsBase {
	///
	MathMetricsBase();

	///
	LyXFont font;
	///
	MathStyles style;
	///
	string fontname;
	/// if this is set...
	bool restrictwidth;
	/// ... this is valid
	int textwidth;
};


struct MathMetricsInfo {
	///
	MathMetricsInfo();

	///
	MathMetricsBase base;
	///
	bool fullredraw;
};


struct MathPainterInfo {
	///
	MathPainterInfo(Painter & pain);
	///
	void draw(int x, int y, char c);

	///
	MathMetricsBase base;
	///
	Painter & pain;
};


struct TextMetricsInfo {};


// Generic base for temporarily changing things.
// The original state gets restored when the Changer is destructed
template <class Struct, class Temp = Struct>
struct MathChanger {
	///
	MathChanger(Struct & orig) : orig_(orig) {}
protected:
	///
	Struct & orig_;
	///
	Temp save_;
};



struct MathFontChanger : public MathChanger<LyXFont> {
	///
	MathFontChanger(LyXFont & orig, char const * font);
	///
	~MathFontChanger();
};


struct MathFontSetChanger : public MathChanger<MathMetricsBase> {
	///
	MathFontSetChanger(MathMetricsBase & mb, char const * font);
	///
	~MathFontSetChanger();
};


struct MathStyleChanger : public MathChanger<MathMetricsBase> {
	///
	MathStyleChanger(MathMetricsBase & mb, MathStyles shape);
	///
	~MathStyleChanger();
};


struct MathScriptChanger : public MathStyleChanger {
	///
	MathScriptChanger(MathMetricsBase & mb);
};


struct MathFracChanger : public MathStyleChanger {
	///
	MathFracChanger(MathMetricsBase & mb);
};


struct MathShapeChanger : public MathChanger<LyXFont, LyXFont::FONT_SHAPE> {
	///
	MathShapeChanger(LyXFont & font, LyXFont::FONT_SHAPE shape);
	///
	~MathShapeChanger();
};


struct MathWidthChanger : public MathChanger<MathMetricsBase>
{
	///
	MathWidthChanger(MathMetricsBase & mb, int width);
	///
	~MathWidthChanger();
};


#endif
