// -*- C++ -*-
/**
 * \file math_support.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SUPPORT_H
#define MATH_SUPPORT_H

#include "LString.h"

class PainterInfo;
class LyXFont;
class Dimension;
class MathArray;

void mathed_char_dim(LyXFont const &, unsigned char c, Dimension & dim);
int mathed_char_width(LyXFont const &, unsigned char c);
int mathed_char_ascent(LyXFont const &, unsigned char c);
int mathed_char_descent(LyXFont const &, unsigned char c);

void mathed_draw_deco(PainterInfo & pi, int x, int y, int w, int h,
	string const & name);

void mathed_string_dim(LyXFont const & font, string const & s, Dimension & dim);
int mathed_string_width(LyXFont const &, string const & s);

void drawStr(PainterInfo & pi,
	LyXFont const &, int x, int y, string const & s);
void drawStrRed(PainterInfo & pi, int x, int y, string const & s);
void drawStrBlack(PainterInfo & pi, int x, int y, string const & s);
void drawChar(PainterInfo & pi, LyXFont const & font, int x, int y, char c);

void math_font_max_dim(LyXFont const &, int & asc, int & desc);

void augmentFont(LyXFont & f, string const & cmd);

bool isFontName(string const & name);

// converts single cell to string
string asString(MathArray const & ar);
// converts string to single cell
void asArray(string const & str, MathArray & ar);

#endif
