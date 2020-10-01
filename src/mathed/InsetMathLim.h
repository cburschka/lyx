// -*- C++ -*-
/**
 * \file InsetMathLim.h
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

#include "InsetMathNest.h"


namespace lyx {

class InsetMathLim : public InsetMathNest {
public:
	///
	InsetMathLim(Buffer * buf, MathData const & f, MathData const & x,
		MathData const & x0);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;

	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void write(WriteStream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_LIM_CODE; }

private:
	Inset * clone() const override;
};



} // namespace lyx
#endif
