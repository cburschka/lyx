#ifndef MATHAUTOCORRECT_H
#define MATHAUTOCORRECT_H

class MathAtom;

// make "corrections" according to file lib/autocorrect
bool math_autocorrect(MathAtom & at, char c);

#endif
