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
	InsetMathPar() {}
	///
	InsetMathPar(MathData const & ar);
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void infoize(odocstream & os) const;
	///
	void write(WriteStream & os) const;
private:
	///
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
