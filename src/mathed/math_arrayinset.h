// -*- C++ -*-
#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathArrayInset : public MathGridInset {
public: 
	///
	MathArrayInset(int m, int n);
	///
	MathArrayInset(int m, int n, char valign, string const & halign);
	/// convienience constructor from whitespace/newline seperated data
	MathArrayInset(string const & str);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	MathArrayInset * asArrayInset() { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
};

#endif
