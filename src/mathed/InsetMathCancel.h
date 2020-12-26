// -*- C++ -*-
/**
 * \file InsetMathCancel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CANCELINSET_H
#define MATH_CANCELINSET_H

#include "InsetMathNest.h"


namespace lyx {

class InsetMathCancel : public InsetMathNest {
public:
	///
	enum Kind {
		cancel,
		bcancel,
		xcancel
	};
	///
	explicit InsetMathCancel(Buffer * buf, Kind);
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
	InsetCode lyxCode() const override { return MATH_CANCEL_CODE; }
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures &) const override;

private:
	///
	Inset * clone() const override;
	///
	Kind kind_;
};



} // namespace lyx
#endif
