// -*- C++ -*-
/**
 * \file InsetMathDelim.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "InsetMathNest.h"


namespace lyx {

/// A delimiter like '{' or '(' or '<'
class InsetMathDelim : public InsetMathNest {
public:
	///
	InsetMathDelim(Buffer * buf, docstring const & left, docstring const & right);
	///
	InsetMathDelim(Buffer * buf, docstring const & left, docstring const & right,
		MathData const &);
	///
	InsetMathDelim * asDelimInset() override { return this; }
	///
	InsetMathDelim const * asDelimInset() const override { return this; }
	///
	MathClass mathClass() const override { return MC_INNER; }
	/// is it (...)?
	bool isParenthesis() const;
	/// is it [...]?
	bool isBrackets() const;
	/// is it |...|?
	bool isAbs() const;
	///
	mode_type currentMode() const override { return MATH_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;

	///
	void validate(LaTeXFeatures & features) const override;
	///
	void write(TeXMathStream & os) const override;
	/// write normalized content
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	InsetCode lyxCode() const override { return MATH_DELIM_CODE; }
	///
	docstring left_;
	///
	docstring right_;
private:
	Inset * clone() const override;
	///
	mutable int dw_;
};

} // namespace lyx

#endif
