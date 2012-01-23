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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
	///
	InsetCode lyxCode() const { return MATH_CANCEL_CODE; }
	/// Nothing for now
	void mathmlize(MathStream &) const {}
	/// Nothing for HTML
	void htmlize(HtmlStream &) const {}
	///
	void validate(LaTeXFeatures &) const;

private:
	///
	virtual Inset * clone() const;
	///
	Kind kind_;
};



} // namespace lyx
#endif
