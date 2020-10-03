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
	explicit InsetMathOverset(Buffer * buf) : InsetMathFracBase(buf) {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	bool idxUpDown(Cursor & cur, bool up) const override;
	///
	bool idxFirst(Cursor &) const override;
	///
	bool idxLast(Cursor &) const override;
	///
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_OVERSET_CODE; }

private:
	Inset * clone() const override;
};



} // namespace lyx
#endif
