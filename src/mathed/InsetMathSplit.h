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
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;

	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	int defaultColSpace(col_type) { return 0; }
	///
	char defaultColAlign(col_type);
	///
	char displayColAlign(idx_type idx) const;
	///
	InsetCode lyxCode() const { return MATH_SPLIT_CODE; }

private:
	///
	virtual Inset * clone() const;
	///
	docstring name_;
	///
	bool numbered_;
};


} // namespace lyx
#endif
