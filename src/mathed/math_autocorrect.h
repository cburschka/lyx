#ifndef MATHAUTOCORRECT_H
#define MATHAUTOCORRECT_H

#ifdef __GNUG__
#pragma interface
#endif

class MathAtom;

// make "corrections" according to file lib/autocorrect
bool math_autocorrect(MathAtom & at, char c);

#endif
