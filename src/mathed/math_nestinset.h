#ifndef MATH_NESTINSET_H
#define MATH_NESTINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_diminset.h"

/** Abstract base class for all math objects that conatin nested items.
*/


class LaTeXFeatures;

class MathNestInset : public MathDimInset {
public: 
	///
	explicit MathNestInset(int na = 0, string const & nm = string());

	///
	void metrics(MathStyles st) const;
	/// draw the object, sets xo_ and yo_ cached values 
	void draw(Painter &, int x, int y) const;
	/// appends itself with macro arguments substituted
	void substitute(MathArray & array, MathMacro const & macro) const;

	/// The left key
	bool idxLeft(int & idx, int & pos) const;
	/// The right key
	bool idxRight(int & idx, int & pos) const;

	/// Move one physical cell up
	bool idxNext(int & idx, int & pos) const;
	/// Move one physical cell down
	bool idxPrev(int & idx, int & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(int & idx, int & pos) const;

	/// Where should we go if we press home?
	bool idxHome(int & idx, int & pos) const;
	/// Where should we go if we press end?
	bool idxEnd(int & idx, int & pos) const;

	///
	int nargs() const;

	///
	MathArray & cell(int);
	///
	MathArray const & cell(int) const;
	///
	MathXArray & xcell(int);
	///
	MathXArray const & xcell(int) const;
			
	///
	bool isActive() const { return nargs() > 0; }
	///
	void push_back(MathInset *);
	///
	void push_back(unsigned char ch, MathTextCodes fcode);
	///
	void dump() const;

	///
	void validate(LaTeXFeatures & features) const;

protected:
	///
	typedef std::vector<MathXArray> cells_type;
	/// The nested contents of the inset are contained here.
	cells_type cells_;
};

#endif
