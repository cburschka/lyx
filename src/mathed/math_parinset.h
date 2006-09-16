// -*- C++ -*-
/**
 * \file math_parinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H


#include "math_hullinset.h"

class InsetMathPar : public InsetMathHull {
public:
	///
	InsetMathPar() {}
	///
	InsetMathPar(MathArray const & ar);
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
private:
	///
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
