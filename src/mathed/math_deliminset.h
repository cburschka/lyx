// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_nestinset.h"
#include "LString.h"


/** A delimiter
 *  \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
*/

class MathDelimInset : public MathNestInset {
public:
	///
	MathDelimInset(string const & left, string const & right);
	///
	MathDelimInset(string const & left, string const & right, MathArray const &);
	///
	MathInset * clone() const;
	///
	MathDelimInset * asDelimInset() { return this; }
	///
	MathDelimInset const * asDelimInset() const { return this; }
	/// is it (...)?
	bool isParanthesis() const;
	/// is it [...]?
	bool isBrackets() const;
	/// is it |...|?
	bool isAbs() const;
	///
	Dimension metrics(MetricsInfo & mi) const;
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
	string left_;
	///
	string right_;
private:
	///
	mutable int dw_;
};
#endif
