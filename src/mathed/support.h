// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "lyxfont.h"
#include "math_defs.h"

#ifndef byte
#define byte unsigned char
#endif

class Painter;
class MathArray;
class MathMatrixInset;

struct math_deco_struct {
	int code;
	float const * data;
	int angle;
};

extern char const * math_font_name[];
extern char const * latex_mathspace[];

int mathed_char_height(short type, int size, byte c, int & asc, int & des);
int mathed_char_width(short type, int size, byte c);
void mathed_char_dim(short type, int size, byte c, int & asc, int & des, int & wid);

void mathed_draw_deco(Painter & pain, int x, int y, int w, int h, int code);

LyXFont mathed_get_font(short type, int size);

void mathed_string_dim(short type, int size, string const & s,
  int & asc, int & des, int & wid);
int mathed_string_height(short type, int size, string const & s,
  int & asc, int & des);
int mathed_string_width(short type, int size, string const & s);

math_deco_struct const * search_deco(int code);

bool MathIsInset(short x);
bool MathIsAlphaFont(short x);
bool MathIsBOPS(short x);
bool MathIsBinary(short x);
bool MathIsSymbol(short x);
bool MathIsRelOp(byte c, MathTextCodes f);

void drawStr(Painter & pain, short type, int siz,
	int x, int y, string const & s);
void drawChar(Painter & pain, short type, int siz,
	int x, int y, char c);
#endif
