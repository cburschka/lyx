// -*- C++ -*-
#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "math_nestinset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for \scriptsize etc
    \author André Poenitz
*/

class MathSizeInset : public MathNestInset {
public:
	///
	explicit MathSizeInset(MathStyles st);
	///
	MathInset * clone() const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;

private:
	///
	char const * verbose() const;
	/// 
	MathStyles style_;
};

#endif
