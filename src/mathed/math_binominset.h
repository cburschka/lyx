// -*- C++ -*-
/**
 * \file math_binominset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BINOMINSET_H
#define MATH_DINOMINSET_H

#include "math_fracbase.h"


/// Binom like objects
class MathBinomInset : public MathFracbaseInset {
public:
	///
	explicit MathBinomInset(bool choose = false);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
private:
	///
	int dw() const;
	///
	bool choose_;
};

#endif
