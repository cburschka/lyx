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
	//void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void substitute(MathArray & array, MathMacro const & macro) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

private:
	/// A number between 1 and 9
	int number_;
	///
	char str_[3];
};

#endif
