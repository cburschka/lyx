// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The different kinds of ellipsis
class MathDotsInset : public MathInset {
public:
	///
	MathDotsInset(string const &, int);
	///
	MathInset *  clone() const;
	///
	void draw(Painter &, int, int);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st);
protected:
	///
	int dh_;
	///
	int code_;
};   
#endif
