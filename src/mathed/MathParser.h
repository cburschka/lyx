// -*- C++ -*-
/**
 * \file MathParser.h
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

#include "support/types.h"
#include "support/docstring.h"


namespace lyx {

class MathAtom;
class MathData;
class InsetMathGrid;
class Lexer;


///
class latexkeys {
public:
	/// name of the macro or primitive
	docstring name;
	/// name of a inset that handles that macro
	docstring inset;
	/**
	 * The string or symbol to draw.
	 * This is a string of length 1 if \p name is a known symbol, and
	 * the corresponding font is available. In this case it is
	 * NO UCS4 STRING! The only "character" of the string simply denotes
	 * the code point of the symbol in the font. Therefore you have to
	 * be very careful if you pass \c draw to any function that takes a
	 * docstring argument.
	 * If \p name is a known symbol, but the corresponding font is not
	 * available, or if it is a function name, then \c draw contains a
	 * regular UCS4 string (actuallay \c draw == \c name) that is painted
	 * on screen.
	 */
	docstring draw;
	/// operator/..., fontname e
	docstring extra;
	/// how is this called as XML entity in MathML?
	docstring xmlname;
	/// required LaTeXFeatures
	docstring requires;
};


/// check whether this is a well-known (La)TeX macro or primitive
latexkeys const * in_word_set(docstring const & str);

/// tell the parser whether it should warn about unusual contents
void mathed_parser_warn_contents(bool);

/// parse formula from a string
bool mathed_parse_normal(MathAtom &, docstring const &);
/// ... the LyX lexxer
bool mathed_parse_normal(MathAtom &, Lexer &);
/// parse formula from a string into a grid
void mathed_parse_normal(InsetMathGrid &, docstring const &);

/// parse a single cell from a string
void mathed_parse_cell(MathData & ar, docstring const &);
/// parse a single cell from a stream. Only use this for reading from .lyx
/// file format, for the reason see Parser::tokenize(std::istream &).
void mathed_parse_cell(MathData & ar, std::istream &);

void initParser();


} // namespace lyx

#endif
