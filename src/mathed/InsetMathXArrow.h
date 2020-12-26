// -*- C++ -*-
/**
 * \file InsetMathXArrow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_XARROWINSET_H
#define MATH_XARROWINSET_H

#include "InsetMathFrac.h"


namespace lyx {


/// Wide arrows like \xrightarrow
class InsetMathXArrow : public InsetMathFracBase {
public:
	///
	explicit InsetMathXArrow(Buffer * buf, docstring const & name);
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void normalize(NormalStream & os) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_XARROW_CODE; }
	///
	MathClass mathClass() const override { return MC_REL; }

private:
	Inset * clone() const override;
	///
	bool upper() const;
	///
	docstring const name_;
};


} // namespace lyx
#endif
