// -*- C++ -*-
#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Decorations and accents over (below) a math object
    \author Alejandro Aguilar Sierra
 */

class MathDecorationInset : public MathNestInset {
public:
	///
	explicit MathDecorationInset(string const & name);
	///
	MathInset * clone() const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void normalize(NormalStream & os) const;
	///
	bool isScriptable() const;

private:
	///
	bool upper() const;
	///
	bool protect() const;
	/// is it a wide decoration?
	bool wide() const;

	///
	string const name_;
	/// height cache of deco
	mutable int dh_;
	/// vertical offset cache of deco
	mutable int dy_;
	/// width for non-wide deco
	mutable int dw_;
};
#endif
