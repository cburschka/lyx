#ifndef MATH_FACTORY_H
#define MATH_FACTORY_H


#include "LString.h"

class MathAtom;
class MathArray;

MathAtom createMathInset(string const &);

/** Fills ar with the contents of str.
 *  str is created by the frontend dialog's and returned to the LyX core.
 *  The function returns true if successful.
 */
bool createMathInset_fromDialogStr(string const &, MathArray &);

#endif
