// -*- C++ -*-
#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A macro argument
 *  \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
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
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
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
};

#endif
