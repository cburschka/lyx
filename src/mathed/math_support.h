// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "LString.h"

class MathPainterInfo;
class TextPainter;
class latexkeys;
class MathMetricsInfo;
class MathInset;
class LyXFont;

void mathed_char_dim(LyXFont const &, unsigned char c,
	int & asc, int & des, int & wid);
int mathed_char_width(LyXFont const &, unsigned char c);
int mathed_char_ascent(LyXFont const &, unsigned char c);
int mathed_char_descent(LyXFont const &, unsigned char c);

void mathed_draw_deco(MathPainterInfo & pain, int x, int y, int w, int h,
	string const & name);

void mathed_draw_framebox(MathPainterInfo & pain, int x, int y, MathInset const *);

void mathed_string_dim(LyXFont const &,
	string const & s, int & asc, int & des, int & wid);

int mathed_string_width(LyXFont const &, string const & s);
int mathed_string_ascent(LyXFont const &, string const & s);
int mathed_string_descent(LyXFont const &, string const & s);

void drawStr(MathPainterInfo & pain,
	LyXFont const &, int x, int y, string const & s);
void drawStrRed(MathPainterInfo & pain, int x, int y, string const & s);
void drawStrBlack(MathPainterInfo & pain, int x, int y, string const & s);
void drawChar(MathPainterInfo & pain,
	LyXFont const & font, int x, int y, char c);

void math_font_max_dim(LyXFont const &, int & asc, int & desc);

void augmentFont(LyXFont & f, string const & cmd);

bool isFontName(string const & name);

#endif
