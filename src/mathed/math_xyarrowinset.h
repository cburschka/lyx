// -*- C++ -*-
#ifndef MATH_XYARROWINSET_H
#define MATH_ARROWINSET_H

#include "math_nestinset.h"
#include "math_metricsinfo.h"

#ifdef __GNUG__
#pragma interface
#endif

// for the \ar stuff in \xymatrix

class MathXYMatrixInset;

class MathXYArrowInset : public MathNestInset {
public:
	///
	MathXYArrowInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	MathXYArrowInset * asXYArrowInset() { return this; }

	///
	void normalize();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;

public:
	///
	MathXYMatrixInset const * targetMatrix() const;
	///
	MathArray const & targetCell() const;
	///
	MathArray const & sourceCell() const;

	///
	bool up_;
	///
	mutable MathMetricsInfo mi_;
	///
	mutable LyXFont font_;
	///
	mutable MathXYMatrixInset const * target_;
};

#endif
