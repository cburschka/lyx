#ifndef MATH_METRICSINFO_H
#define MATH_METRICSINFO_H

#include "lyxfont.h"
#include "math_defs.h"

class BufferView;
class Painter;
class MathNestInset;


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
	bool fontinset;
};


struct MathMetricsInfo {
	///
	MathMetricsInfo();

	///
	MathMetricsBase base;
	///
	BufferView * view;
	/// used to pass some info down
	MathNestInset const * inset;
	///
	int idx;
	///
	bool fullredraw;
};


struct MathPainterInfo {
	///
	MathPainterInfo(Painter & p);
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



struct MathScriptChanger : public MathChanger<MathMetricsBase> {
	///
	MathScriptChanger(MathMetricsBase & orig);
	///
	~MathScriptChanger();
};


struct MathFracChanger : public MathChanger<MathMetricsBase> {
	///
	MathFracChanger(MathMetricsBase & orig);
	///
	~MathFracChanger();
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


struct MathShapeChanger : public MathChanger<LyXFont, LyXFont::FONT_SHAPE> {
	///
	MathShapeChanger(LyXFont & font, LyXFont::FONT_SHAPE shape);
	///
	~MathShapeChanger();
};


#endif
