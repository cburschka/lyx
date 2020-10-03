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
	bool extraBraces() const override { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;

	///
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	InsetCode lyxCode() const override { return MATH_SIZE_CODE; }

private:
	Inset * clone() const override;
	///
	latexkeys const * key_;
	///
	MathStyle const style_;
};



} // namespace lyx
#endif
