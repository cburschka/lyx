#ifndef MATH_NESTINSET_H
#define MATH_NESTINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_diminset.h"

/** Abstract base class for all math objects that contain nested items.
    This is basically everything that is not a single character or a
    single symbol.
*/


class LaTeXFeatures;

class MathNestInset : public MathDimInset {
public:
	/// nestinsets have a fixed size to start with
	explicit MathNestInset(idx_type ncells);

	/// the size is usuall some sort of convex hull of the cells
	void metrics(MathMetricsInfo const & mi) const;
	/// add space for markers
	void metricsMarkers(int frame = 1) const;
	/// add space for markers
	void metricsMarkers2(int frame = 1) const;
	/// draw background if locked
	void draw(MathPainterInfo & pi, int x, int y) const;
	/// draw selection background
	void drawSelection(MathPainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const;
	void drawMarkers(MathPainterInfo & pi, int x, int y) const;
	/// draw four angular markers
	void drawMarkers2(MathPainterInfo & pi, int x, int y) const;
	/// appends itself with macro arguments substituted
	void substitute(MathMacro const & macro);
	/// identifies NestInsets
	MathNestInset * asNestInset() { return this; }
	/// get cursor position
	void getPos(idx_type idx, pos_type pos, int & x, int & y) const;

	/// order of movement through the cells when pressing the left key
	bool idxLeft(idx_type & idx, pos_type & pos) const;
	/// order of movement through the cells when pressing the right key
	bool idxRight(idx_type & idx, pos_type & pos) const;

	/// move one physical cell up
	bool idxNext(idx_type & idx, pos_type & pos) const;
	/// move one physical cell down
	bool idxPrev(idx_type & idx, pos_type & pos) const;

	/// target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(idx_type & idx, pos_type & pos) const;
	/// target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(idx_type & idx, pos_type & pos) const;

	/// where should we go if we press home?
	bool idxHome(idx_type & idx, pos_type & pos) const;
	/// where should we go if we press end?
	bool idxEnd(idx_type & idx, pos_type & pos) const;

	/// number of cells currently governed by us
	idx_type nargs() const;
	/// access to the lock
	bool lock() const;
	/// access to the lock
	void lock(bool);
	/// get notification when the cursor leaves this inset
	void notifyCursorLeaves();

	/// direct access to the cell
	MathArray & cell(idx_type);
	/// direct access to the cell
	MathArray const & cell(idx_type) const;
	/// direct access to the cell including the drawing cache
	MathXArray & xcell(idx_type);
	/// direct access to the cell including the drawing cache
	MathXArray const & xcell(idx_type) const;

	/// can we move into this cell (see macroarg.h)
	bool isActive() const;
	/// request "external features"
	void validate(LaTeXFeatures & features) const;

	/// match in all cells
	bool match(MathInset *) const;
	/// replace in all cells
	void replace(ReplaceData &);
	/// do we contain a given pattern?
	bool contains(MathArray const &);
	/// glue everything to a single cell
	MathArray glue() const;

	/// debug helper
	void dump() const;
	/// is the cursor currently somewhere within this inset?
	virtual bool editing() const;

protected:
	/// we store the cells in a vector
	typedef std::vector<MathXArray> cells_type;
	/// thusly:
	cells_type cells_;
	/// if the inset is locked, it can't be entered with the cursor
	bool lock_;
};

#endif
