// -*- C++ -*-
#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for super- and subscripts
    \author André Pönitz
 */


class MathScriptInset : public MathNestInset {
public:
	///
	MathScriptInset();
	///
	MathScriptInset(bool up, bool down, MathInset * = 0);
	///
	MathScriptInset(MathScriptInset const &);
	///
	~MathScriptInset();
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	bool idxUp(int & idx, int & pos) const;
	///
	bool idxDown(int & idx, int & pos) const;
	///
	bool idxLeft(int & idx, int & pos) const;
	///
	bool idxRight(int & idx, int & pos) const;
	///
	bool idxFirst(int & idx, int & pos) const;
	///
	bool idxFirstUp(int & idx, int & pos) const;
	///
	bool idxFirstDown(int & idx, int & pos) const;
	///
	bool idxLast(int & idx, int & pos) const;
	///
	bool idxLastUp(int & idx, int & pos) const;
	///
	bool idxLastDown(int & idx, int & pos) const;
	///
	void idxDelete(int & idx, bool & popit, bool & deleteit);

	///
	bool up() const;
	///
	bool down() const;
	///
	void up(bool);
	///
	void down(bool);
	///
	void limits(int);
	///
	int limits() const;
	///
	bool isActive() const { return false; }
	/// Identifies ScriptInsets
	bool isScriptInset() const { return true; }
	///
	int xoffset() const { return dxx_; }
private:
	///
	bool hasLimits() const;
	///
	bool up_;
	///
	bool down_;
	/// 1: \limits, -1: \nolimits, 0: use default
	int limits_;
	/// x offset cache for drawing the superscript
	mutable int dx0_;
	/// x offset cache for drawing the subscript
	mutable int dx1_;
	/// x offset cache for drawing the inner symbol
	mutable int dxx_;
	/// y offset cache for drawing the superscript
	mutable int dy0_;
	/// y offset cache for drawing the subscript
	mutable int dy1_;
	///
	MathInset * symbol_;
};

#endif
