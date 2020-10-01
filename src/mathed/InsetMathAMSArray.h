// -*- C++ -*-
/**
 * \file InsetMathAMSArray.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_AMSARRAYINSET_H
#define MATH_AMSARRAYINSET_H

#include "InsetMathGrid.h"


namespace lyx {

/// Inset for things like [pbvV]matrix, psmatrix etc
class InsetMathAMSArray : public InsetMathGrid {
public:
	///
	InsetMathAMSArray(Buffer * buf, docstring const &, int m, int n);
	///
	InsetMathAMSArray(Buffer * buf, docstring const &);	///

	///
	int rowsep() const override;
	///
	int colsep() const override;
	///
	int border() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pain, int x, int y) const override;
	///
	InsetMathAMSArray * asAMSArrayInset() override { return this; }
	///
	InsetMathAMSArray const * asAMSArrayInset() const override { return this; }

	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;
	///
	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream &) const override;
	// Don't need mathmlize or htmlize, as this is handled by
	// InsetMathMatrix after being extracted in MathExtern.
	// void mathmlize(MathStream &) const override;
	// void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_AMSARRAY_CODE; }
	///
	char const * name_left() const;
	///
	char const * name_right() const;
	///
	int leftMargin() const override { return small() ? 3 : 6; }
	///
	int rightMargin() const override { return small() ? 3: 6; }
	///
	bool handlesMulticolumn() const override { return true; }

private:
	Inset * clone() const override;
	///
	bool small() const { return name_ == "smallmatrix"; }
	///
	docstring name_;
};

} // namespace lyx

#endif
