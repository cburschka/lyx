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

#include <string>
#include <vector>

namespace lyx {


class MathAtom;
class MathArray;
class InsetMathGrid;
class LyXLex;


///
class latexkeys {
public:
	/// name of the macro or primitive
	docstring name;
	/// name of a inset that handles that macro
	docstring inset;
	/// position of the thing in a font	
	docstring draw;
	/// operator/..., fontname e
	docstring extra;
	/// how is this called as XML entity?
	docstring xmlname;
	/// required LaTeXFeatures
	docstring requires;
};


/// check whether this is a well-known (La)TeX macro or primitive
latexkeys const * in_word_set(docstring const & str);

/// parse formula from a string
bool mathed_parse_normal(MathAtom &, docstring const &);
/// ... a stream
bool mathed_parse_normal(MathAtom &, std::istream &);
/// ... the LyX lexxer
bool mathed_parse_normal(MathAtom &, LyXLex &);
/// ... the LyX lexxer
void mathed_parse_normal(InsetMathGrid &, docstring const &);

/// parse a single cell from a string
void mathed_parse_cell(MathArray & ar, docstring const &);
/// ... a stream
void mathed_parse_cell(MathArray & ar, std::istream &);

void initParser();


} // namespace lyx

#endif
