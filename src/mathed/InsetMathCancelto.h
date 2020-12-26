// -*- C++ -*-
/**
 * \file InsetMathCancelto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CANCELTOINSET_H
#define MATH_CANCELTOINSET_H

#include "InsetMathNest.h"


namespace lyx {

class InsetMathCancelto : public InsetMathNest {
public:
	///
	explicit InsetMathCancelto(Buffer * buf);
	///
	bool idxUpDown(Cursor & cur, bool up) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_CANCELTO_CODE; }
	/// Nothing for now
	void mathmlize(MathMLStream &) const override {}
	/// Nothing for HTML
	void htmlize(HtmlStream &) const override {}
	///
	void validate(LaTeXFeatures &) const override;

private:
	///
	Inset * clone() const override;
};



} // namespace lyx
#endif
