// -*- C++ -*-

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "lyxfont.h"

#ifndef byte
#define byte unsigned char
#endif

class Painter;
class MathedArray;
class MathParInset;

struct math_deco_struct {
	int code;
	float const * data;
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

/// math_parser.C
extern
void mathed_parse(MathedArray & data, MathParInset * & par, unsigned flags);

/// math_parser.C
extern
void mathed_parser_file(std::istream &, int);
/// math_parser.C
extern
int mathed_parser_lineno();

extern
bool MathIsInset(short x);

extern
bool MathIsFont(short x);

extern
bool MathIsAlphaFont(short x);

extern
bool MathIsActive(short x);

extern
bool MathIsUp(short x);

extern
bool MathIsDown(short x);

extern
bool MathIsScript(short x);

extern
bool MathIsBOPS(short x);

extern
bool MathIsBinary(short x);

extern
bool MathIsSymbol(short x);

extern
bool is_eqn_type(short int type);

extern
bool is_matrix_type(short int type);

extern
bool is_multiline(short int type);

extern
bool is_ams(short int type);

extern
bool is_singlely_numbered(short int type);

extern
bool is_multi_numbered(short int type);

extern
bool is_numbered(short int type);

extern
bool is_multicolumn(short int type);

#endif
