// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "math_defs.h"
#include "LString.h"

class Painter;
class latexkeys;
class MathMetricsInfo;
class MathInset;

extern char const * latex_mathspace[];

int mathed_char_height(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c, int & asc, int & des);
void mathed_char_dim(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c, int & asc, int & des, int & wid);
int mathed_char_width(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c);
int mathed_char_ascent(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c);
int mathed_char_descent(MathTextCodes type, MathMetricsInfo const & size,
	unsigned char c);

void mathed_draw_deco(Painter & pain, int x, int y, int w, int h,
	string const & name);

void mathed_draw_framebox(Painter & pain, int x, int y, MathInset const *);

void mathed_string_dim(MathTextCodes type, MathMetricsInfo const & size,
	string const & s, int & asc, int & des, int & wid);
int mathed_string_height(MathTextCodes type, MathMetricsInfo const & size,
	string const & s, int & asc, int & des);

int mathed_string_width(MathTextCodes type, MathMetricsInfo const & size,
	string const & s);
int mathed_string_ascent(MathTextCodes type, MathMetricsInfo const & size,
	string const & s);
int mathed_string_descent(MathTextCodes type, MathMetricsInfo const & size,
	string const & s);

void drawStr(Painter & pain, MathTextCodes type, MathMetricsInfo const & siz,
	int x, int y, string const & s);
void drawChar(Painter & pain, MathTextCodes type, MathMetricsInfo const & siz,
	int x, int y, char c);

void math_font_max_dim(MathTextCodes code, MathMetricsInfo const & siz,
	int & asc, int & desc);

bool math_font_available(MathTextCodes code);

// decrease math size for super- and subscripts
void smallerStyleScript(MathMetricsInfo &);

// decrease math size for fractions
void smallerStyleFrac(MathMetricsInfo & st);

char const * math_font_name(MathTextCodes type);


#endif
