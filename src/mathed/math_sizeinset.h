// -*- C++ -*-
#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for \scriptsize etc
    \author André Pönitz
*/

class latexkeys;

class MathSizeInset : public MathNestInset {
public:
	///
	explicit MathSizeInset(latexkeys const * l);
	///
	MathInset * clone() const;
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	latexkeys const * key_;
	///
	MathStyles const style_;
};

#endif
