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
	MathInset * Clone() const;
	///
	void Metrics(MathStyles st);
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;

private:
	///
	char const * verbose() const;
	/// 
	MathStyles style_;
};

#endif
