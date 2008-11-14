// -*- C++ -*-
/**
 * \file InsetMathDiff.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DIFFINSET_H
#define MATH_DIFFINSET_H


// d f(x)/dx in one block
// for interfacing external programs

#include "InsetMathNest.h"


namespace lyx {

class InsetMathDiff : public InsetMathNest {
public:
	///
	InsetMathDiff();
	///
	void addDer(MathData const & der);
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
	void mathmlize(MathStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void write(WriteStream & os) const;
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
