// -*- C++ -*-
/**
 * \file InsetMathEnsureMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ENSUREMATHINSET_H
#define MATH_ENSUREMATHINSET_H

#include "InsetMathGrid.h"


namespace lyx {


/// Inset for ensuring math mode
class InsetMathEnsureMath : public InsetMathGrid {
public:
	explicit InsetMathEnsureMath(Buffer * buf);
	///
	mode_type currentMode() const override { return MATH_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter & pi, int x, int y) const override;
	///
	void addRow(row_type row) override {}
	///
	void delRow(row_type row) override {}
	///
	void swapRow(row_type row) override {}
	///
	void addCol(col_type col) override {}
	///
	void delCol(col_type col) override {}
	///
	docstring eolString(row_type row, bool fragile, bool latex,
	                    bool last_eoln) const override
	{ return docstring(); }
	///
	docstring eocString(col_type col, col_type lastcol) const override
	{ return docstring(); }
	///
	void write(TeXMathStream & os) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_ENSUREMATH_CODE; }
private:
	Inset * clone() const override;
};


} // namespace lyx

#endif
