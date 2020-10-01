// -*- C++ -*-
/**
 * \file InsetMathArray.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "InsetMathGrid.h"


namespace lyx {


/// Inset for things like \begin{array}...\end{array}
class InsetMathArray : public InsetMathGrid {
public:
	///
	InsetMathArray(Buffer * buf, docstring const &, int m, int n);
	///
	InsetMathArray(Buffer * buf, docstring const &, int m, int n,
		char valign, docstring const & halign);
	/// convenience constructor from whitespace/newline separated data
	InsetMathArray(Buffer * buf, docstring const &, docstring const & str);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	InsetMathArray * asArrayInset() override { return this; }
	///
	InsetMathArray const * asArrayInset() const override { return this; }

	///
	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream & os) const override;
	///
	void maple(MapleStream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_ARRAY_CODE; }
	///
	int leftMargin() const override { return 4; }
	///
	int rightMargin() const override { return 2; }
	///
	bool handlesMulticolumn() const override { return true; }

private:
	Inset * clone() const override;
	///
	docstring name_;
};


} // namespace lyx

#endif
