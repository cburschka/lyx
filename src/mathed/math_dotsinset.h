// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The different kinds of ellipsis
class MathDotsInset : public MathedInset {
public:
	///
	MathDotsInset(string const &, int, short st = LM_ST_TEXT);
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
protected:
	///
	int dh_;
	///
	int code_;
};   
#endif
