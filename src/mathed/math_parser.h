// -*- C++ -*-
/*
 *  File:        math_parser.h
 *  Purpose:     Declaration of parsing utilities for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Parse LaTeX2e math mode code.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_PARSER_H
#define MATH_PARSER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "math_defs.h"
#include "symbol_def.h"

class MathArray;
class MathMatrixInset;
class MathMacroTemplate;
class LyXLex;

///
enum MathTokenEnum
{
	///
	LM_TK_SYM = 256,
	///
	LM_TK_CHOOSE,
	///
	LM_TK_BINOM,
	///
	LM_TK_ATOP,
	///
	LM_TK_OVER,
	///
	LM_TK_FRAC,
	///
	LM_TK_SQRT,
	///
	LM_TK_ROOT,
	///
	LM_TK_BEGIN,
	///
	LM_TK_END,
	///
	LM_TK_NEWLINE,
	///
	LM_TK_UNDEF,
	/// mathcal, mathrm...
	LM_TK_OLDFONT,
	/// cal,...
	LM_TK_FONT,
	///
	LM_TK_LEFT,
	///
	LM_TK_RIGHT,
	///
	LM_TK_DECORATION,
	///
	LM_TK_FUNC,
	///
	LM_TK_FUNCLIM,
	///
	LM_TK_BIGSYM,
	///
	LM_TK_NOGLYPH,
	///
	LM_TK_NOGLYPHB,
	///
	LM_TK_CMSY,
	///
	LM_TK_CMM,
	///
	LM_TK_CMEX,
	///
	LM_TK_MSA,
	///
	LM_TK_MSB,
	///
	LM_TK_LABEL,
	///
	LM_TK_NONUM,
	///
	LM_TK_SPACE,
	///
	LM_TK_DOTS,
	///
	LM_TK_LIMIT,
	///
	LM_TK_PROTECT,
	///
	LM_TK_STY,
	///
	LM_TK_SPECIAL,
	///
	LM_TK_ARGUMENT, 
	///
	LM_TK_NEWCOMMAND,
	///
	LM_TK_MATH,
	///
	LM_TK_NOT,
	///
	LM_TK_KERN,
	///
	LM_TK_STACK
};


///
struct latexkeys {
	///
	string name;
	///
	short token;
	///
	unsigned int id;
	///
	unsigned char latex_font_id;
	///
	MathSymbolTypes type;
	///
	string const & Name() const { return name;}
};


///
latexkeys const * in_word_set(string const & str);

///
void ReadSymbols(string const & file);

MathMatrixInset * mathed_parse_normal(string const &);
MathMatrixInset * mathed_parse_normal(std::istream &);
MathMatrixInset * mathed_parse_normal(LyXLex &);

MathMacroTemplate * mathed_parse_macro(string const &);
MathMacroTemplate * mathed_parse_macro(std::istream &);
MathMacroTemplate * mathed_parse_macro(LyXLex &);

#endif
