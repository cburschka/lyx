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
	MathInset *  Clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
protected:
	///
	int dh_;
	///
	int code_;
};   
#endif
