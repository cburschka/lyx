// -*- C++ -*-
/**
 * \file math_nestinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_NESTINSET_H
#define MATH_NESTINSET_H

#include "math_diminset.h"


/** Abstract base class for all math objects that contain nested items.
    This is basically everything that is not a single character or a
    single symbol.
*/

class MathNestInset : public MathDimInset {
public:
	/// nestinsets have a fixed size to start with
	explicit MathNestInset(idx_type ncells);

	/// the size is usually some sort of convex hull of the cells
	/// hides inset::metrics() intentionally!
	void metrics(MetricsInfo const & mi) const;
	/// draw background if locked
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw selection background
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// appends itself with macro arguments substituted
	void substitute(MathMacro const & macro);
	/// identifies NestInsets
	MathNestInset * asNestInset() { return this; }
	/// identifies NestInsets
	MathNestInset const * asNestInset() const { return this; }
	/// get cursor position
	void getCursorPos(CursorSlice const & cur, int & x, int & y) const;
	///
	void edit(LCursor & cur, bool left);
	///
	InsetBase * editXY(LCursor & cur, int x, int y);

	/// order of movement through the cells when pressing the left key
	bool idxLeft(LCursor &) const;
	/// order of movement through the cells when pressing the right key
	bool idxRight(LCursor &) const;

	/// move one physical cell up
	bool idxNext(LCursor &) const;
	/// move one physical cell down
	bool idxPrev(LCursor &) const;

	/// target pos when we enter the inset from the left by pressing "Right"
	bool idxFirst(LCursor &) const;
	/// target pos when we enter the inset from the right by pressing "Left"
	bool idxLast(LCursor &) const;

	/// number of cells currently governed by us
	idx_type nargs() const;
	/// access to the lock
	bool lock() const;
	/// access to the lock
	void lock(bool);
	/// get notification when the cursor leaves this inset
	void notifyCursorLeaves(idx_type);

	/// direct access to the cell
	MathArray & cell(idx_type);
	/// direct access to the cell
	MathArray const & cell(idx_type) const;

	/// can we move into this cell (see macroarg.h)
	bool isActive() const;
	/// request "external features"
	void validate(LaTeXFeatures & features) const;

	/// replace in all cells
	void replace(ReplaceData &);
	/// do we contain a given pattern?
	bool contains(MathArray const &) const;
	/// glue everything to a single cell
	MathArray glue() const;

	/// debug helper
	void dump() const;

	/// writes \\, name(), and args in braces and '\\lyxlock' if necessary
	void write(WriteStream & os) const;
	/// writes [, name(), and args in []
	void normalize(NormalStream & os) const;

protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
	///
	void handleFont(LCursor & cur,
		std::string const & arg, std::string const & font);
	///
	void handleFont2(LCursor & cur, std::string const & arg);

	///
	bool interpret(LCursor & cur, char c);
	///
	bool script(LCursor & cur, bool);


private:
	/// lfun handler
	void lfunMousePress(LCursor &, FuncRequest &);
	///
	void lfunMouseRelease(LCursor &, FuncRequest &);
	///
	void lfunMouseMotion(LCursor &, FuncRequest &);

protected:
	/// we store the cells in a vector
	typedef std::vector<MathArray> cells_type;
	/// thusly:
	cells_type cells_;
	/// if the inset is locked, it can't be entered with the cursor
	bool lock_;
};

#endif
