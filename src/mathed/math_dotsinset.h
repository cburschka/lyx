// -*- C++ -*-
#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_diminset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The different kinds of ellipsis
class MathDotsInset : public MathDimInset {
public:
	///
	explicit MathDotsInset(string const &);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
protected:
	/// cache for the thing's heigth
	mutable int dh_;
	///
	string const name_;
};   
#endif
