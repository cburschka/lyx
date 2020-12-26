// -*- C++ -*-
/**
 * \file InsetMathCases.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CASESINSET_H
#define MATH_CASESINSET_H

#include "InsetMathGrid.h"


namespace lyx {


class LaTeXFeatures;

class InsetMathCases : public InsetMathGrid {
public:
	///
	explicit InsetMathCases(Buffer * buf, row_type rows = 1u);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;

	///
	void infoize(odocstream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_CASES_CODE; }
	///
	int displayColSpace(col_type) const override;
	///
	int leftMargin() const override { return 8; }
	///
	int rightMargin() const override { return 0; }
	/// see e.g. https://tex.stackexchange.com/a/133283/87201
	bool handlesMulticolumn() const override { return true; }

private:
	Inset * clone() const override;
};


} // namespace lyx

#endif
