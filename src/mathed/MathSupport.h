// -*- C++ -*-
/**
 * \file MathSupport.h
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

#include "support/docstring.h"

#include <string>
#include <vector>

namespace lyx {

class PainterInfo;
class LyXFont;
class Dimension;
class MathArray;
class MathAtom;
class InsetMath;


int mathed_char_width(LyXFont const &, char_type c);

void mathed_draw_deco(PainterInfo & pi, int x, int y, int w, int h,
	docstring const & name);

void mathed_string_dim(LyXFont const & font,
		       docstring const & s,
		       Dimension & dim);

int mathed_string_width(LyXFont const &, docstring const & s);

void drawStrRed(PainterInfo & pi, int x, int y, docstring const & s);
void drawStrBlack(PainterInfo & pi, int x, int y, docstring const & s);

void math_font_max_dim(LyXFont const &, int & asc, int & desc);

void augmentFont(LyXFont & f, docstring const & cmd);

bool isFontName(docstring const & name);

// converts single cell to string
docstring asString(MathArray const & ar);
// converts single inset to string
docstring asString(InsetMath const &);
docstring asString(MathAtom const &);
// converts string to single cell
void asArray(docstring const & str, MathArray & ar);

} // namespace lyx

#endif
