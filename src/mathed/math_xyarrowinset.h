// -*- C++ -*-
/**
 * \file math_xyarrowinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_XYARROWINSET_H
#define MATH_ARROWINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


// for the \ar stuff in \xymatrix

class MathXYMatrixInset;

class MathXYArrowInset : public MathNestInset {
public:
	///
	MathXYArrowInset();
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
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
	mutable MetricsInfo mi_;
	///
	mutable LyXFont font_;
	///
	mutable MathXYMatrixInset const * target_;
};

#endif
