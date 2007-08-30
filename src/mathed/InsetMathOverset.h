// -*- C++ -*-
/**
 * \file InsetMathOverset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_OVERSETINSET_H
#define MATH_OVERSETINSET_H


#include "InsetMathFracBase.h"


namespace lyx {

/// Inset for overset
class InsetMathOverset : public InsetMathFracBase {
public:
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool idxFirst(Cursor &) const;
	///
	bool idxLast(Cursor &) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
