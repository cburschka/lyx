// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_diminset.h"


class latexkeys;

/// The different kinds of ellipsis
class MathDotsInset : public MathDimInset {
public:
	///
	explicit MathDotsInset(latexkeys const * l);
	///
	MathInset * clone() const;
	///
	void metrics(MetricsInfo & mi) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	string name() const;
protected:
	/// cache for the thing's heigth
	mutable int dh_;
	///
	latexkeys const * key_;
};
#endif
