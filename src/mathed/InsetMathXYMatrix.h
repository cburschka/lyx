// -*- C++ -*-
/**
 * \file InsetMathXYMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_XYMATRIX_H
#define MATH_XYMATRIX_H

#include "InsetMathGrid.h"

#include "support/Length.h"


namespace lyx {


class InsetMathXYMatrix : public InsetMathGrid {
public:
	///
	InsetMathXYMatrix(Buffer * buf, Length const & = Length(), char c = '\0',
		bool equal_spacing = false);
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	InsetMathXYMatrix const * asXYMatrixInset() const { return this; }
	///
	int colsep() const override;
	///
	int rowsep() const override;

	///
	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_XYMATRIX_CODE; }
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;

private:
	///
	Inset * clone() const override;
	/// extra spacing, may be empty
	Length spacing_;
	///
	char spacing_code_;
	///
	bool equal_spacing_;
};



} // namespace lyx
#endif
