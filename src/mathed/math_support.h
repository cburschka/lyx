// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "math_defs.h"
#include "LString.h"

class Painter;
class TextPainter;
class latexkeys;
class MathMetricsInfo;
class MathInset;
class LyXFont;

extern char const * latex_mathspace[];

void mathed_char_dim(LyXFont const &, unsigned char c,
	int & asc, int & des, int & wid);
int mathed_char_height(LyXFont const &, unsigned char c, int & asc, int & des);
int mathed_char_width(LyXFont const &, unsigned char c);
int mathed_char_ascent(LyXFont const &, unsigned char c);
int mathed_char_descent(LyXFont const &, unsigned char c);

void mathed_draw_deco(Painter & pain, int x, int y, int w, int h,
	string const & name);

void mathed_draw_framebox(Painter & pain, int x, int y, MathInset const *);

void mathed_string_dim(LyXFont const &,
	string const & s, int & asc, int & des, int & wid);
int mathed_string_height(LyXFont const &,
	string const & s, int & asc, int & des);

int mathed_string_width(LyXFont const &, string const & s);
int mathed_string_ascent(LyXFont const &, string const & s);
int mathed_string_descent(LyXFont const &, string const & s);

void drawStr(Painter & pain, LyXFont const &, int x, int y, string const & s);
void drawChar(Painter & pain, LyXFont const & font, int x, int y, char c);

void math_font_max_dim(LyXFont const &, int & asc, int & desc);

bool math_font_available(MathTextCodes code);

// decrease math size for super- and subscripts
void smallerStyleScript(MathMetricsInfo &);

// decrease math size for fractions
void smallerStyleFrac(MathMetricsInfo & st);

char const * math_font_name(MathTextCodes type);

string convertDelimToLatexName(string const & name);

void whichFont(LyXFont & f, MathTextCodes type, MathMetricsInfo const & size);

#endif
