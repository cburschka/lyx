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

#include "MathParser_flags.h"
#include "support/strfwd.h"

namespace lyx {

class MetricsInfo;
class PainterInfo;
class FontInfo;
class Dimension;
class MathData;
class MathAtom;
class InsetMath;


int mathed_font_em(FontInfo const &);

int mathed_char_width(FontInfo const &, char_type c);

int mathed_char_kerning(FontInfo const &, char_type c);

void mathed_draw_deco(PainterInfo & pi, int x, int y, int w, int h,
	docstring const & name);

void mathed_string_dim(FontInfo const & font,
		       docstring const & s,
		       Dimension & dim);

int mathed_string_width(FontInfo const &, docstring const & s);

void metricsStrRedBlack(MetricsInfo & mi, Dimension & dim, docstring const & s);

void drawStrRed(PainterInfo & pi, int x, int y, docstring const & s);
void drawStrBlack(PainterInfo & pi, int x, int y, docstring const & s);

void math_font_max_dim(FontInfo const &, int & asc, int & desc);

void augmentFont(FontInfo & f, docstring const & cmd);

bool isFontName(docstring const & name);

bool isMathFont(docstring const & name);

bool isTextFont(docstring const & name);

bool isAlphaSymbol(MathAtom const & at);

// converts single cell to string
docstring asString(MathData const & ar);
// converts single inset to string
docstring asString(InsetMath const &);
docstring asString(MathAtom const &);
// converts string to single cell
void asArray(docstring const &, MathData &, Parse::flags f = Parse::NORMAL);

} // namespace lyx

#endif
