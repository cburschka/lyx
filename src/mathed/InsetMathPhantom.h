// -*- C++ -*-
/**
 * \file InsetMathPhantom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PHANTOMINSET_H
#define MATH_PHANTOMINSET_H

#include "InsetMathNest.h"


namespace lyx {

class InsetMathPhantom : public InsetMathNest {
public:
	///
	enum Kind {
		phantom,
		vphantom,
		hphantom,
		smash,
		smasht,
		smashb,
		mathclap,
		mathllap,
		mathrlap
	};
	///
	explicit InsetMathPhantom(Buffer * buf, Kind);
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
	InsetCode lyxCode() const override { return MATH_PHANTOM_CODE; }
	/// Nothing for now
	void mathmlize(MathMLStream &) const override {}
	/// Nothing for HTML
	void htmlize(HtmlStream &) const override {}
	/// request "external features"
	void validate(LaTeXFeatures & features) const override;
	/// Does the contents appear in LaTeX output?
	bool visibleContents() const;

private:
	///
	Inset * clone() const override;
	///
	Kind kind_;
};



} // namespace lyx
#endif
