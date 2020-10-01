// -*- C++ -*-
/**
 * \file InsetMathTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_TABULARINSET_H
#define MATH_TABULARINSET_H

#include "InsetMathGrid.h"


namespace lyx {


/// Inset for things like \begin{tabular}...\end{tabular}
class InsetMathTabular : public InsetMathGrid {
public:
	///
	InsetMathTabular(Buffer * buf, docstring const &, int m, int n);
	///
	InsetMathTabular(Buffer * buf, docstring const &, int m, int n,
		char valign, docstring const & halign);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	InsetMathTabular * asTabularInset() { return this; }
	///
	InsetMathTabular const * asTabularInset() const { return this; }

	///
	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	InsetCode lyxCode() const override { return MATH_TABULAR_CODE; }
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
