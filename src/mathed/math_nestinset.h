#ifndef MATH_NESTINSET_H
#define MATH_NESTINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_inset.h"

/** Abstract base class for all math objects that conatin nested items.
*/


class LaTeXFeatures;

class MathNestInset : public MathInset {
public: 
	///
	explicit MathNestInset(int na = 0, string const & nm = string());

	/// draw the object, sets xo_ and yo_ cached values 
	virtual void draw(Painter &, int x, int baseline);
	/// appends itself with macro arguments substituted
	virtual void substitute(MathArray & array, MathMacro const & macro) const;
	/// compute the size of the object, sets ascend_, descend_ and width_
	virtual void metrics(MathStyles st) = 0;

	/// The left key
	virtual bool idxLeft(int & idx, int & pos) const;
	/// The right key
	virtual bool idxRight(int & idx, int & pos) const;

	/// Move one physical cell up
	virtual bool idxNext(int & idx, int & pos) const;
	/// Move one physical cell down
	virtual bool idxPrev(int & idx, int & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	virtual bool idxFirst(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(int & idx, int & pos) const;

	/// Where should we go if we press home?
	virtual bool idxHome(int & idx, int & pos) const;
	/// Where should we go if we press end?
	virtual bool idxEnd(int & idx, int & pos) const;

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
