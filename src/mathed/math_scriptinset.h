// -*- C++ -*-
#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for super- and subscripts
    \author André Pönitz
 */


class MathScriptInset : public MathInset {
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
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
	///
	void draw(Painter &, int x, int baseline);

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
	///
	string ssym_;
	/// 1: \limits, -1: \nolimits, 0: use default
	int limits_;
	/// x offset for drawing the superscript
	int dx0_;
	/// x offset for drawing the subscript
	int dx1_;
	/// x offset for drawing the inner symbol
	int dxx_;
	///
	int dy0_;
	///
	int dy1_;
	///
	MathInset * symbol_;
};

#endif
