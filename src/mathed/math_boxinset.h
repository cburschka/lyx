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
	explicit MathBoxInset(string const & name);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	/// identifies BoxInsets
	MathBoxInset * asBoxInset() { return this; }
	/// identifies BoxInsets
	MathBoxInset const * asBoxInset() const { return this; }
	///
	void rebreak();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	mutable MathMetricsInfo mi_;
	///
	string name_;
};


#endif
