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
	explicit MathNestInset(unsigned int ncells);

	///
	void metrics(MathStyles st) const;
	/// draw the object, sets xo_ and yo_ cached values 
	void draw(Painter &, int x, int y) const;
	/// appends itself with macro arguments substituted
	void substitute(MathMacro const & macro); 

	/// The left key
	bool idxLeft(unsigned int & idx, unsigned int & pos) const;
	/// The right key
	bool idxRight(unsigned int & idx, unsigned int & pos) const;

	/// Move one physical cell up
	bool idxNext(unsigned int & idx, unsigned int & pos) const;
	/// Move one physical cell down
	bool idxPrev(unsigned int & idx, unsigned int & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(unsigned int & idx, unsigned int & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(unsigned int & idx, unsigned int & pos) const;

	/// Where should we go if we press home?
	bool idxHome(unsigned int & idx, unsigned int & pos) const;
	/// Where should we go if we press end?
	bool idxEnd(unsigned int & idx, unsigned int & pos) const;

	///
	unsigned int nargs() const;

	///
	MathArray & cell(unsigned int);
	///
	MathArray const & cell(unsigned int) const;
	///
	MathXArray & xcell(unsigned int);
	///
	MathXArray const & xcell(unsigned int) const;
			
	///
	bool isActive() const { return nargs() > 0; }
	///
	void push_back(MathInset *);
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
