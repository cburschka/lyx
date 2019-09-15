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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool idxUpDown(Cursor & cur, bool up) const;
	///
	bool idxFirst(Cursor &) const;
	///
	bool idxLast(Cursor &) const;
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
