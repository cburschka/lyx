// -*- C++ -*-
#ifndef MATH_LEFTEQNINSET_H
#define MATH_LEFTEQNINSET_H

#include "math_nestinset.h"


/// Support for LaTeX's \\lefteqn command

class MathLefteqnInset : public MathNestInset {
public:
	///
	MathLefteqnInset();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	string name() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
};
#endif
