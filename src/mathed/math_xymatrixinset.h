// -*- C++ -*-
#ifndef MATH_XYMATRIX_H
#define MATH_XYMATRIX_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathXYMatrixInset : public MathGridInset {
public:
	///
	MathXYMatrixInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	MathXYMatrixInset const * asXYMatrixInset() const { return this; }
	///
	virtual int colsep() const;
	///
	virtual int rowsep() const;

	///
	void normalize();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
};

#endif
