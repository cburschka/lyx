// -*- C++ -*-
#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "math_nestinset.h"
#include "math_defs.h"

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
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;

private:
	/// 
	latexkeys const * key_;
};

#endif
