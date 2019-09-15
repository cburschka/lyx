// -*- C++ -*-
/**
 * \file InsetMathUnderset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_UNDERSETINSET_H
#define MATH_UNDERSETINSET_H


#include "InsetMathFrac.h"


namespace lyx {

/// Inset for underset
class InsetMathUnderset : public InsetMathFracBase {
public:
	///
	explicit InsetMathUnderset(Buffer * buf) : InsetMathFracBase(buf) {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool idxFirst(Cursor &) const;
	///
	bool idxLast(Cursor &) const;
	///
	void write(WriteStream & ws) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_UNDERSET_CODE; }

private:
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
