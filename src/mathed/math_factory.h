#ifndef MATH_FACTORY_H
#define MATH_FACTORY_H

#include "LString.h"

class MathInset;
class latexkeys;

MathInset * createMathInset(string const &);
MathInset * createMathInset(latexkeys const *);

#endif
