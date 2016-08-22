// -*- C++ -*-
/**
 * \file MathClass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CLASS_H
#define MATH_CLASS_H

#include "support/strfwd.h"

namespace lyx {

class MetricsBase;

/* The TeXbook, p. 158:
 *
 * There are thirteen kinds of atoms, each of which might act
 * differently in a formula; for example, ‘(’ is an Open atom because
 * it comes from an opening. Here is a complete list of the different
 * kinds:

 * + Ord: an ordinary atom like ‘x’
 * + Op: a large operator atom like ‘\sum’
 * + Bin: a binary operation atom like ‘+’
 * + Rel: a relation atom like ‘=’
 * + Open: an opening atom like ‘(’
 * + Close: a closing atom like ‘)’
 * + Punct: a punctuation atom like ‘,’
 * + Inner: an inner atom like ‘\frac{1}{2}’
 * + Over: an overline atom like ‘\overline{x}’
 * + Under: an underline atom like ‘\underline{x}’
 * + Acc: an accented atom like ‘\hat{x}’
 * + Rad: a radical atom like ‘\sqrt{2}’
 * + Vcent: a vbox to be centered, produced by \vcenter.
 *
 * Over, Under, Acc, Rad and Vcent are not considered in the enum
 * below. The relvant elements will be considered as Ord.
 */
enum MathClass {
	MC_ORD,
	MC_OP,
	MC_BIN,
	MC_REL,
	MC_OPEN,
	MC_CLOSE,
	MC_PUNCT,
	MC_INNER,
	MC_UNKNOWN
};


MathClass string_to_class(docstring const &);

docstring const class_to_string(MathClass);

void update_class(MathClass & mc, MathClass const prev, MathClass const next);

int class_spacing(MathClass const mc1, MathClass const mc2,
                  MetricsBase const & mb);

} // namespace lyx

#endif
