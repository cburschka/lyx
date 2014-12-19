// -*- C++ -*-
/**
 * \file InsetMathNest.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_NESTINSET_H
#define MATH_NESTINSET_H

#include "InsetMath.h"

#include <map>

namespace lyx {

/** Abstract base class for all math objects that contain nested items.
    This is basically everything that is not a single character or a
    single symbol.
*/

class InsetMathNest : public InsetMath {
public:
	/// nestinsets have a fixed size to start with
	InsetMathNest(Buffer * buf, idx_type ncells);
	///
	virtual ~InsetMathNest();
	///
	void setBuffer(Buffer &);

	// The method below hides inset::metrics() intentionally!
	// We have to tell clang not to be fussy about that.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
	/// the size is usually some sort of convex hull of the cells
	void metrics(MetricsInfo const & mi) const;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	/// draw background if locked
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw selection background
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// draw decorations.
	void drawDecoration(PainterInfo & pi, int x, int y) const
	{ drawMarkers(pi, x, y); }
	///
	void updateBuffer(ParIterator const &, UpdateType);
	/// identifies NestInsets
	InsetMathNest * asNestInset() { return this; }
	/// identifies NestInsets
	InsetMathNest const * asNestInset() const { return this; }
	/// get cursor position
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	void edit(Cursor & cur, bool front, 
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	Inset * editXY(Cursor & cur, int x, int y);

	/// order of movement through the cells when moving backwards
	bool idxBackward(Cursor &) const;
	/// order of movement through the cells when moving forward
	bool idxForward(Cursor &) const;

	/// move to next cell
	bool idxNext(Cursor &) const;
	/// move to previous cell
	bool idxPrev(Cursor &) const;

	/// target pos when we enter the inset while moving forward
	bool idxFirst(Cursor &) const;
	/// target pos when we enter the inset while moving backwards
	bool idxLast(Cursor &) const;

	/// number of cells currently governed by us
	idx_type nargs() const;
	/// access to the lock
	bool lock() const;
	/// access to the lock
	void lock(bool);
	/// get notification when the cursor leaves this inset
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	//@{
	/// direct access to the cell.
	/// Inlined because of performance reasons.
	MathData & cell(idx_type i) { return cells_[i]; }
	MathData const & cell(idx_type i) const { return cells_[i]; }
	//@}

	/// can we move into this cell (see macroarg.h)
	bool isActive() const;
	/// request "external features"
	void validate(LaTeXFeatures & features) const;

	/// replace in all cells
	void replace(ReplaceData &);
	/// do we contain a given pattern?
	bool contains(MathData const &) const;
	/// glue everything to a single cell
	MathData glue() const;

	/// debug helper
	void dump() const;

	/// writes \\, name(), and args in braces and '\\lyxlock' if necessary
	void write(WriteStream & os) const;
	/// writes [, name(), and args in []
	void normalize(NormalStream & os) const;
	///
	void latex(otexstream & os, OutputParams const & runparams) const;
	///
	bool setMouseHover(BufferView const * bv, bool mouse_hover) const;
	///
	bool mouseHovered(BufferView const * bv) const 
		{ return mouse_hover_[bv]; }

	///
	bool completionSupported(Cursor const &) const;
	///
	bool inlineCompletionSupported(Cursor const & cur) const;
	///
	bool automaticInlineCompletion() const;
	///
	bool automaticPopupCompletion() const;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const;
	///
	docstring completionPrefix(Cursor const & cur) const;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished);
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const;
	///
	InsetCode lyxCode() const { return MATH_NEST_CODE; }

protected:
	///
	InsetMathNest(InsetMathNest const & inset);
	///
	InsetMathNest & operator=(InsetMathNest const &);

	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;
	///
	void handleFont(Cursor & cur,
		docstring const & arg, docstring const & font);
	void handleFont(Cursor & cur,
		docstring const & arg, char const * const font);
	///
	void handleFont2(Cursor & cur, docstring const & arg);
	/// Grab and erase selection and insert the InsetMathNest atom in every
	/// previously selected cell, insert the grabbed former data and \c arg
	/// in the first cell of the inserted atom.
	void handleNest(Cursor & cur, MathAtom const & nest);
	void handleNest(Cursor & cur, MathAtom const & nest, docstring const & arg);

	/// interpret \p c and insert the result at the current position of
	/// of \p cur. Return whether the cursor should stay in the formula.
	bool interpretChar(Cursor & cur, char_type c);
	///
	bool script(Cursor & cur, bool);
	bool script(Cursor & cur, bool, docstring const & save_selection);

public:
	/// interpret \p str and insert the result at the current position of
	/// \p cur if it is something known. Return whether \p cur was
	/// inserted.
	virtual bool interpretString(Cursor & cur, docstring const & str);

private:
	/// lfun handler
	void lfunMousePress(Cursor &, FuncRequest &);
	///
	void lfunMouseRelease(Cursor &, FuncRequest &);
	///
	void lfunMouseMotion(Cursor &, FuncRequest &);
	/// Find a macro to fold or unfold, starting at searchCur and searchCur.nextInset() pointing to a macro
	/// afterwards if found
	bool findMacroToFoldUnfold(Cursor & searchCur, bool fold) const;
	/// move cursor forward
	bool cursorMathForward(Cursor & cur);
	/// move cursor backwards
	bool cursorMathBackward(Cursor & cur);

protected:
	/// we store the cells in a vector
	typedef std::vector<MathData> cells_type;
	/// thusly:
	cells_type cells_;
	/// if the inset is locked, it can't be entered with the cursor
	bool lock_;
	///
	mutable std::map<BufferView const *, bool> mouse_hover_;
};	



} // namespace lyx
#endif
