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
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo & st) const;
protected:
	/// cache for the thing's heigth
	mutable int dh_;
	///
	latexkeys const * key_;
};
#endif
