// -*- C++ -*-
/**
 * \file InsetMathDots.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "InsetMath.h"


namespace lyx {

class latexkeys;

/// The different kinds of ellipsis
class InsetMathDots : public InsetMath {
public:
	///
	explicit InsetMathDots(latexkeys const * l);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	docstring name() const override;
	/// request "external features"
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_DOTS_CODE; }
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & os) const override;
protected:
	/// cache for the thing's height
	mutable int dh_;
	///
	latexkeys const * key_;
private:
	Inset * clone() const override;
};

} // namespace lyx

#endif
