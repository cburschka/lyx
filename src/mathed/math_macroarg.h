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
class MathMacroArgument : public MathedInset {
public:
	///
	explicit MathMacroArgument(int);
	///
	MathedInset * Clone();
	///
	void substitute(MathMacro *);
	///
	void Metrics();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	///
	int number() const;


private:
	/// A number between 1 and 9
	int number_;
};

#endif
