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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;

	///
	void infoize(odocstream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void write(WriteStream & os) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_CASES_CODE; }
	///
	int displayColSpace(col_type) const;
	///
	int leftMargin() const { return 8; } //override
	///
	int rightMargin() const { return 0; } //override

private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
