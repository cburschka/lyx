// -*- C++ -*-
/**
 * \file InsetMathSplit.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPLITINSET_H
#define MATH_SPLITINSET_H

#include "InsetMathGrid.h"


namespace lyx {


class InsetMathSplit : public InsetMathGrid {
public:
	///
	explicit InsetMathSplit(Buffer * buf, docstring const & name,
		char valign = 'c', bool numbered = false);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;

	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	int defaultColSpace(col_type) override { return 0; }
	///
	int displayColSpace(col_type col) const override;
	///
	char defaultColAlign(col_type) override;
	///
	char displayColAlign(idx_type idx) const override;
	///
	InsetCode lyxCode() const override { return MATH_SPLIT_CODE; }

private:
	///
	Inset * clone() const override;
	///
	docstring name_;
	///
	bool numbered_;
};


} // namespace lyx
#endif
