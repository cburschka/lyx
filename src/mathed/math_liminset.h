// -*- C++ -*-
/**
 * \file math_liminset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_LIMINSET_H
#define MATH_LIMINSET_H


// lim_{x->x0} f(x) in one block
// for interfacing external programs

#include "math_nestinset.h"

class MathLimInset : public MathNestInset {
public:
	///
	MathLimInset(MathArray const & f, MathArray const & x, MathArray const & x0);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;
};

#endif
