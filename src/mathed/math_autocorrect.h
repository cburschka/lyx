// -*- C++ -*-
/**
 * \file math_autocorrect.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATHAUTOCORRECT_H
#define MATHAUTOCORRECT_H


class MathAtom;

// make "corrections" according to file lib/autocorrect
bool math_autocorrect(MathAtom & at, char c);

#endif
