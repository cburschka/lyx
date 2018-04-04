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


#include "InsetMathFrac.h"


namespace lyx {

/// Inset for overset
class InsetMathOverset : public InsetMathFracBase {
public:
	///
	InsetMathOverset(Buffer * buf) : InsetMathFracBase(buf) {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	idx_type firstIdx() const { return 1; }
	///
	idx_type lastIdx() const { return 1; }
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_OVERSET_CODE; }

private:
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
