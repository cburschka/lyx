// -*- C++ -*-
/**
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

class MathAtom;
class MathArray;
class MathGridInset;
class LyXLex;


///
struct latexkeys {
	/// name of the macro or primitive
	string name;
	/// name of a inset that handles that macro
	string inset;
	/// position of the ting in a font
	string draw;
	/// operator/..., fontname e
	string extra;
	/// how is this called as XML entity?
	string xmlname;
};


/// check whether this is a well-known (La)TeX macro or primitive
latexkeys const * in_word_set(string const & str);

/// parse formula from a string
bool mathed_parse_normal(MathAtom &, string const &);
/// ... a stream
bool mathed_parse_normal(MathAtom &, std::istream &);
/// ... the LyX lexxer
bool mathed_parse_normal(MathAtom &, LyXLex &);
/// ... the LyX lexxer
void mathed_parse_normal(MathGridInset &, string const &);

/// parse a single cell from a string
void mathed_parse_cell(MathArray & ar, string const &);
/// ... a stream
void mathed_parse_cell(MathArray & ar, std::istream &);

#endif
