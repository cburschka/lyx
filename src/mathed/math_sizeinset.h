// -*- C++ -*-
#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for \scriptsize etc
    \author André Poenitz
*/

class MathSizeInset : public MathInset {
public:
	///
	explicit MathSizeInset(MathStyles st);
	///
	virtual MathInset * clone() const;
	///
	void metrics(MathStyles st);
	///
	void draw(Painter &, int x, int baseline);
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
