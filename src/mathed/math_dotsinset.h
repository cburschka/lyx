// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

class latexkeys;

/// The different kinds of ellipsis
class MathDotsInset : public MathDimInset {
public:
	///
	explicit MathDotsInset(latexkeys const * l);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	string name() const;
protected:
	/// cache for the thing's heigth
	mutable int dh_;
	///
	latexkeys const * key_;
};
#endif
