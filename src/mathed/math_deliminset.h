// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A delimiter
    \author Alejandro Aguilar Sierra
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
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	string left_;
	///
	string right_;
private:
	///
	int dw() const;
};
#endif
