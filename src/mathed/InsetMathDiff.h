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
	explicit InsetMathDiff(Buffer * buf);
	///
	void addDer(MathData const & der);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;

	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_DIFF_CODE; }
private:
	Inset * clone() const override;
};


} // namespace lyx

#endif
