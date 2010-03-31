// -*- C++ -*-
/**
 * \file InsetMathSize.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATHSIZEINSET_H
#define MATHSIZEINSET_H

#include "InsetMathNest.h"
#include "MetricsInfo.h"


namespace lyx {


class latexkeys;

/// An inset for \scriptsize etc.
class InsetMathSize : public InsetMathNest {
public:
	///
	explicit InsetMathSize(Buffer * buf, latexkeys const * l);
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(odocstream & os) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetCode lyxCode() const { return MATH_SIZE_CODE; }

private:
	virtual Inset * clone() const;
	///
	latexkeys const * key_;
	///
	Styles const style_;
};



} // namespace lyx
#endif
