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

#include <string>


namespace lyx {

/// A delimiter like '{' or '(' or '<'
class InsetMathDelim : public InsetMathNest {
public:
	///
	InsetMathDelim(docstring const & left, docstring const & right);
	///
	InsetMathDelim(docstring const & left, docstring const & right, MathData const &);
	///
	InsetMathDelim * asDelimInset() { return this; }
	///
	InsetMathDelim const * asDelimInset() const { return this; }
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
	void octave(OctaveStream &) const;
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
