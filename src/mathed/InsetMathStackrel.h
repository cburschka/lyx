// -*- C++ -*-
/**
 * \file InsetMathStackrel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_STACKRELINSET_H
#define MATH_STACKRELINSET_H

#include "InsetMathFrac.h"


namespace lyx {

class InsetMathStackrel : public InsetMathFracBase {
public:
	///
	InsetMathStackrel(Buffer * buf, bool sub);
	///
	bool idxUpDown(Cursor &, bool up) const override;
	///
	bool idxFirst(Cursor &) const override;
	///
	bool idxLast(Cursor &) const override;
	///
	MathClass mathClass() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	InsetCode lyxCode() const override { return MATH_STACKREL_CODE; }

private:
	Inset * clone() const override;
};

} // namespace lyx
#endif
