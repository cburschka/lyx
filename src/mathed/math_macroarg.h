// -*- C++ -*-
#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "math_nestinset.h"


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
	InsetBase * clone() const;
	///
	bool isActive() const { return false; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
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
