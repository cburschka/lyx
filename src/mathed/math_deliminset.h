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
	MathDelimInset(string const &, string const &);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	/// write normalized content
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	string octavize() const;
	///
	string maplize() const;
private:
	///
	int dw() const;
	///
	string left_;
	///
	string right_;
	///
	static string latexName(string const & name);
};
#endif
