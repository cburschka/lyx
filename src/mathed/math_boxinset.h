// -*- C++ -*-
#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "math_gridinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXFont;

/// Support for \\mbox

class MathBoxInset : public MathGridInset {
public:
	///
	explicit MathBoxInset(string const &);
	///
	MathInset * clone() const;
	/// identifies BoxInsets
	MathBoxInset * asBoxInset() { return this; }
	/// identifies BoxInsets
	MathBoxInset const * asBoxInset() const { return this; }
	///
	void rebreak();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

private:
	///
	mutable MathMetricsInfo mi_;
	///
	string name_;
};
#endif
