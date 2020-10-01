// -*- C++ -*-
/**
 * \file InsetMathPar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H


#include "InsetMathHull.h"


namespace lyx {

class InsetMathPar : public InsetMathHull {
public:
	///
	explicit InsetMathPar(Buffer * buf) : InsetMathHull(buf) {}
	///
	InsetMathPar(Buffer * buf, MathData const & ar);
	///
	mode_type currentMode() const override { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void write(WriteStream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_PAR_CODE; }

private:
	///
	Inset * clone() const override;
};



} // namespace lyx
#endif
