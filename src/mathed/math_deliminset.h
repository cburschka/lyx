// -*- C++ -*-
/**
 * \file math_deliminset.h
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

#include "math_nestinset.h"

#include <string>


/// A delimiter
class MathDelimInset : public MathNestInset {
public:
	///
	MathDelimInset(std::string const & left, std::string const & right);
	///
	MathDelimInset(std::string const & left, std::string const & right, MathArray const &);
	///
	MathDelimInset * asDelimInset() { return this; }
	///
	MathDelimInset const * asDelimInset() const { return this; }
	/// is it (...)?
	bool isParenthesis() const;
	/// is it [...]?
	bool isBrackets() const;
	/// is it |...|?
	bool isAbs() const;
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
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	std::string left_;
	///
	std::string right_;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	mutable int dw_;
};
#endif
