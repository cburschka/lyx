// -*- C++ -*-
#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Smart spaces
class MathSpaceInset : public MathedInset {
public:
	///
	MathSpaceInset(int sp, short ot = LM_OT_SPACE, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void WriteNormal(std::ostream &);
	///
	void Metrics();
	///
	void SetSpace(int sp);
	///
	int GetSpace();
private:
	///
	int space_;
};
#endif
