// -*- C++ -*-
#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A macro argument
    \author Alejandro Aguilar Sierra
*/
class MathMacroArgument : public MathInset {
public:
	///
	explicit MathMacroArgument(int);
	///
	MathInset * clone() const;
	///
	void metrics(MathStyles st);
	///
	void draw(Painter &, int x, int baseline);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void substitute(MathArray & array, MathMacro const & macro) const;

private:
	/// A number between 1 and 9
	int number_;
};

#endif
