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

#include <string>


class MathAtom;
class MathArray;
class MathGridInset;
class LyXLex;


///
class latexkeys {
public:
	/// name of the macro or primitive
	std::string name;
	/// name of a inset that handles that macro
	std::string inset;
	/// position of the ting in a font
	std::string draw;
	/// operator/..., fontname e
	std::string extra;
	/// how is this called as XML entity?
	std::string xmlname;
};


/// check whether this is a well-known (La)TeX macro or primitive
latexkeys const * in_word_set(std::string const & str);

/// parse formula from a string
bool mathed_parse_normal(MathAtom &, std::string const &);
/// ... a stream
bool mathed_parse_normal(MathAtom &, std::istream &);
/// ... the LyX lexxer
bool mathed_parse_normal(MathAtom &, LyXLex &);
/// ... the LyX lexxer
void mathed_parse_normal(MathGridInset &, std::string const &);

/// parse a single cell from a string
void mathed_parse_cell(MathArray & ar, std::string const &);
/// ... a stream
void mathed_parse_cell(MathArray & ar, std::istream &);

void initParser();

#endif
