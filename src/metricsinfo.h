#ifndef METRICSINFO_H
#define METRICSINFO_H

#include "lyxfont.h"
#include "LString.h"

class Painter;
class BufferView;


/// Standard Sizes (mode styles)
enum Styles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


// 
// This is the part common to MetricsInfo and PainterInfo
//
struct MetricsBase {
	///
	MetricsBase();

	/// the current view
	BufferView * bv;
	/// current font
	LyXFont font;
	/// current math style (display/text/script/..)
	Styles style;
	/// name of current font
	string fontname;
	/// if this is set...
	bool restrictwidth;
	/// ... this is valid
	int textwidth;
};


//
// This contains a Metricsbase and Information that's only relevant during
// the first phase of the two-phase draw
//
struct MetricsInfo {
	///
	MetricsInfo();

	///
	MetricsBase base;
};


//
// This contains a Metricsbase and Information that's only relevant during
// the second phase of the two-phase draw
//
struct PainterInfo {
	///
	explicit PainterInfo(BufferView * bv);
	///
	void draw(int x, int y, char c);

	///
	MetricsBase base;
	///
	Painter & pain;
	/// width of current item
	int width;
};


struct TextMetricsInfo {};


// Generic base for temporarily changing things.
// The original state gets restored when the Changer is destructed.

template <class Struct, class Temp = Struct>
struct Changer {
	///
	Changer(Struct & orig) : orig_(orig) {}
protected:
	///
	Struct & orig_;
	///
	Temp save_;
};



// temporarily change some aspect of a font
struct FontChanger : public Changer<LyXFont> {
	///
	FontChanger(LyXFont & orig, char const * font);
	///
	~FontChanger();
};


// temporarily change a full font
struct FontSetChanger : public Changer<MetricsBase> {
	///
	FontSetChanger(MetricsBase & mb, char const * font);
	///
	~FontSetChanger();
};


// temporarily change the style
struct StyleChanger : public Changer<MetricsBase> {
	///
	StyleChanger(MetricsBase & mb, Styles style);
	///
	~StyleChanger();
};


// temporarily change the style to script style
struct ScriptChanger : public StyleChanger {
	///
	ScriptChanger(MetricsBase & mb);
};


// temporarily change the style suitable for use in fractions
struct FracChanger : public StyleChanger {
	///
	FracChanger(MetricsBase & mb);
};


// temporarily change the style suitable for use in tabulars and arrays
struct ArrayChanger : public StyleChanger {
	///
	ArrayChanger(MetricsBase & mb);
};



// temporarily change the shape of a font
struct ShapeChanger : public Changer<LyXFont, LyXFont::FONT_SHAPE> {
	///
	ShapeChanger(LyXFont & font, LyXFont::FONT_SHAPE shape);
	///
	~ShapeChanger();
};


// temporarily change the available text width
struct WidthChanger : public Changer<MetricsBase>
{
	///
	WidthChanger(MetricsBase & mb, int width);
	///
	~WidthChanger();
};


#endif
