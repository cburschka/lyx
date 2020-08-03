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
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	InsetMathXYMatrix const * asXYMatrixInset() const { return this; }
	///
	virtual int colsep() const;
	///
	virtual int rowsep() const;

	///
	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_XYMATRIX_CODE; }
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;

private:
	///
	virtual Inset * clone() const;
	/// extra spacing, may be empty
	Length spacing_;
	///
	char spacing_code_;
	///
	bool equal_spacing_;
};



} // namespace lyx
#endif
