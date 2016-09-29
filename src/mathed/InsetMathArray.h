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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	InsetMathArray * asArrayInset() { return this; }
	///
	InsetMathArray const * asArrayInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	void normalize(NormalStream & os) const;
	///
	void maple(MapleStream & os) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_ARRAY_CODE; }
	///
	int leftMargin() const { return 4; } //override
	///
	int rightMargin() const { return 2; } //override

private:
	virtual Inset * clone() const;
	///
	docstring name_;
};


} // namespace lyx

#endif
