// -*- C++ -*-
/**
 * \file InsetMathSubstack.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SUBSTACK_H
#define MATH_SUBSTACK_H

#include "InsetMathGrid.h"


namespace lyx {


/// support for AMS's \\substack

class InsetMathSubstack : public InsetMathGrid {
public:
	///
	explicit InsetMathSubstack(Buffer * buf);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	InsetMathSubstack const * asSubstackInset() const override { return this; }

	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void write(WriteStream & os) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	InsetCode lyxCode() const override { return MATH_SUBSTACK_CODE; }

private:
	Inset * clone() const override;
};



} // namespace lyx
#endif
