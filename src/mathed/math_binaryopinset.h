#if 0
// -*- C++ -*-
/**
 * \file math_binaryopinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BINARYOPINSET_H
#define MATH_BINARYOPINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


/// An inset for multiplication
class MathBinaryOpInset : public MathNestInset {
public:
	///
	explicit MathBinaryOpInset(char op);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MetricsInfo & st) const;
private:
	///
	int opwidth() const;
	///
	char op_;
	///
	mutable MetricsInfo mi_;
};
#endif
#endif
