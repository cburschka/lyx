// -*- C++ -*-
#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A macro argument
    \author Alejandro Aguilar Sierra
*/
class MathMacroArgument : public MathNestInset {
public:
	///
	explicit MathMacroArgument(int);
	///
	MathInset * clone() const;
	///
	bool isActive() const { return false; }
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void substitute(MathMacro const & macro);

	///
	void normalize(NormalStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// A number between 1 and 9
	int number_;
	///
	char str_[3];
	///
	bool expanded_;
	///
	mutable MathMetricsInfo mi_;
};

#endif
