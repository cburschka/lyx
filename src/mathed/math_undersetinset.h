// -*- C++ -*-
/**
 * \file math_undersetinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_UNDERSETINSET_H
#define MATH_UNDERSETINSET_H


#include "math_fracbase.h"

/// Inset for underset
class MathUndersetInset : public MathFracbaseInset {
public:
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool idxFirst(LCursor & cur) const;
	///
	bool idxLast(LCursor & cur) const;
	///
	bool idxUpDown(LCursor & cur, bool up) const;
	///
	void write(WriteStream & ws) const;
	///
	void normalize(NormalStream & ns) const;
};

#endif
