// -*- C++ -*-
/**
 * \file math_factory.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FACTORY_H
#define MATH_FACTORY_H


#include "support/std_string.h"

class MathAtom;
class MathArray;

MathAtom createMathInset(string const &);

/** Fills ar with the contents of str.
 *  str is created by the frontend dialog's and returned to the LyX core.
 *  The function returns true if successful.
 */
bool createMathInset_fromDialogStr(string const &, MathArray &);

#endif
