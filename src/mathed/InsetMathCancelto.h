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
	bool idxUpDown(Cursor & cur, bool up) const;
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
	InsetCode lyxCode() const { return MATH_CANCELTO_CODE; }
	/// Nothing for now
	void mathmlize(MathStream &) const {}
	/// Nothing for HTML
	void htmlize(HtmlStream &) const {}
	///
	void validate(LaTeXFeatures &) const;

private:
	///
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
