// -*- C++ -*-
/**
 * \file InsetMathBoldSymbol.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BOLDSYMBOLINSET_H
#define MATH_BOLDSYMBOLINSET_H

#include "InsetMathNest.h"


namespace lyx {

/// Inset for AMSTeX's \boldsymbol
class InsetMathBoldSymbol : public InsetMathNest {
public:
	enum Kind {
		AMS_BOLD,
		BM_BOLD,
		BM_HEAVY
	};
	///
	InsetMathBoldSymbol(Buffer * buf, Kind kind = AMS_BOLD);
	///
	docstring name() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter & pi, int x, int y) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_BOLDSYMBOL_CODE; }
	///
	Kind kind_;
private:
	Inset * clone() const override;
};


} // namespace lyx

#endif
