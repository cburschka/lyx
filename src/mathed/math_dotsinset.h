// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

struct latexkeys;

/// The different kinds of ellipsis
class MathDotsInset : public MathDimInset {
public:
	///
	explicit MathDotsInset(latexkeys const *);
	///
	MathInset * clone() const;
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
	latexkeys const * key_;
};   
#endif
