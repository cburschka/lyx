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
	explicit MathNestInset(idx_type ncells);

	///
	void metrics(MathMetricsInfo const & st) const;
	/// draw the object, sets xo_ and yo_ cached values 
	void draw(Painter &, int x, int y) const;
	/// appends itself with macro arguments substituted
	void substitute(MathMacro const & macro); 
	/// identifies NestInsets
	MathNestInset * asNestInset() { return this; }

	/// The left key
	bool idxLeft(idx_type & idx, pos_type & pos) const;
	/// The right key
	bool idxRight(idx_type & idx, pos_type & pos) const;

	/// Move one physical cell up
	bool idxNext(idx_type & idx, pos_type & pos) const;
	/// Move one physical cell down
	bool idxPrev(idx_type & idx, pos_type & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(idx_type & idx, pos_type & pos) const;

	/// Where should we go if we press home?
	bool idxHome(idx_type & idx, pos_type & pos) const;
	/// Where should we go if we press end?
	bool idxEnd(idx_type & idx, pos_type & pos) const;

	///
	idx_type nargs() const;

	///
	MathArray & cell(idx_type);
	///
	MathArray const & cell(idx_type) const;
	///
	MathXArray & xcell(idx_type);
	///
	MathXArray const & xcell(idx_type) const;
			
	///
	bool isActive() const { return nargs() > 0; }
	///
	void push_back(MathAtom const &);
	///
	void dump() const;
	///
	bool match(MathInset *) const;
	///
	void replace(ReplaceData &);

	///
	void validate(LaTeXFeatures & features) const;
	///
	bool covers(int x, int y) const;

protected:
	///
	typedef std::vector<MathXArray> cells_type;
	/// The nested contents of the inset are contained here.
	cells_type cells_;
};

#endif
