// -*- C++ -*-
/**
 * \file math_parser.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PARSER_H
#define MATH_PARSER_H


#include "support/std_string.h"

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
