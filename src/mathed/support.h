// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "math_defs.h"
#include "LString.h"

class Painter;
class latexkeys;

extern char const * math_font_name[];
extern char const * latex_mathspace[];

int mathed_char_height(MathTextCodes type, MathStyles size, unsigned char c,
	int & asc, int & des);
void mathed_char_dim(MathTextCodes type, MathStyles size, unsigned char c,
	int & asc, int & des, int & wid);
int mathed_char_width(MathTextCodes type, MathStyles size, unsigned char c);
int mathed_char_ascent(MathTextCodes type, MathStyles size, unsigned char c);
int mathed_char_descent(MathTextCodes type, MathStyles size, unsigned char c);

void mathed_draw_deco
	(Painter & pain, int x, int y, int w, int h, latexkeys const * l);

void mathed_string_dim(MathTextCodes type, MathStyles size, string const & s,
  int & asc, int & des, int & wid);
int mathed_string_height(MathTextCodes type, MathStyles size, string const & s,
  int & asc, int & des);

int mathed_string_width(MathTextCodes type, MathStyles size, string const & s);
int mathed_string_ascent(MathTextCodes type, MathStyles size, string const & s);
int mathed_string_descent(MathTextCodes type, MathStyles size, string const & s);

bool MathIsAlphaFont(MathTextCodes x);

void drawStr(Painter & pain, MathTextCodes type, MathStyles siz,
	int x, int y, string const & s);
void drawChar(Painter & pain, MathTextCodes type, MathStyles siz,
	int x, int y, char c);

void math_font_max_dim
	(MathTextCodes code, MathStyles siz, int & asc, int & desc);

#endif
