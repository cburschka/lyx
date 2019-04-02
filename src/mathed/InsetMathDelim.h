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
	InsetMathDelim * asDelimInset() { return this; }
	///
	InsetMathDelim const * asDelimInset() const { return this; }
	///
	MathClass mathClass() const { return MC_INNER; }
	/// is it (...)?
	bool isParenthesis() const;
	/// is it [...]?
	bool isBrackets() const;
	/// is it |...|?
	bool isAbs() const;
	///
	mode_type currentMode() const { return MATH_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

	///
	void validate(LaTeXFeatures & features) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	InsetCode lyxCode() const { return MATH_DELIM_CODE; }
	///
	docstring left_;
	///
	docstring right_;
private:
	virtual Inset * clone() const;
	///
	mutable int dw_;
};

} // namespace lyx

#endif
