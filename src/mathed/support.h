// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "lyxfont.h"

#ifndef byte
#define byte unsigned char
#endif

class Painter;

struct math_deco_struct {
	int code;
	float * data;
	int angle;
};

extern char const * math_font_name[];
extern char const * latex_mathspace[];

extern int mathed_char_height(short type, int size, byte c,
			      int & asc, int & des);
extern int mathed_char_width(short type, int size, byte c);
extern void mathed_draw_deco(Painter & pain, int x, int y,
			     int w, int h, int code);

extern LyXFont mathed_get_font(short type, int size);
extern int mathed_string_height(short type, int size, string const & s,
				int & asc, int & des);
extern int mathed_string_width(short type, int size, string const & s);
extern math_deco_struct const * search_deco(int code);
#endif
