// -*- C++ -*-
/**
 * \file math_cursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CURSOR
#define MATH_CURSOR

#include "cursor.h"
#include "math_inset.h"
#include "math_data.h"

#include <string>

class InsetFormulaBase;
class BufferView;
class PainterInfo;
class MathUnknownInset;

/**

[Have a look at math_inset.h first]

The MathCursor is different from the kind of cursor used in the Outer
World. It contains a stack of CursorSlice, each of which is made
up of a inset pointer, an index and a position offset, marking a path from
this formula's MathHullInset to the current position.

*/


namespace mathcursor {
	/// short of anything else reasonable
	typedef size_t             size_type;
	/// type for column numbers
	typedef ptrdiff_t          difference_type;
	/// type for cursor positions within a cell
	typedef lyx::pos_type      pos_type;
	/// type for cell indices
	typedef size_t             idx_type;
	/// type for row numbers
	typedef size_t             row_type;
	/// type for column numbers
	typedef size_t             col_type;

	///
	void insert(LCursor & cur, MathAtom const &);
	///
	void insert(LCursor & cur, MathArray const &);
	///
	void insert2(LCursor & cur, std::string const &);
	///
	void paste(LCursor & cur, std::string const & data);
	/// return false for empty math insets
	bool erase(LCursor & cur);
	/// return false for empty math insets
	bool backspace(LCursor & cur);
	/// called for LFUN_HOME etc
	bool home(LCursor & cur, bool sel = false);
	/// called for LFUN_END etc
	bool end(LCursor & cur, bool sel = false);
	/// called for LFUN_RIGHT and LFUN_RIGHTSEL
	bool right(LCursor & cur, bool sel = false);
	/// called for LFUN_LEFT etc
	bool left(LCursor & cur, bool sel = false);
	/// called for LFUN_UP etc
	bool up(LCursor & cur, bool sel = false);
	/// called for LFUN_DOWN etc
	bool down(LCursor & cur, bool sel = false);
	/// move to next cell in current inset
	void idxNext(LCursor & bv);
	/// move to previous cell in current inset
	void idxPrev(LCursor & bv);
	///
	void plainErase(LCursor & cur);
	///
	void plainInsert(LCursor & cur, MathAtom const & at);
	///
	void niceInsert(LCursor & cur, MathAtom const & at);
	///
	void niceInsert(LCursor & cur, std::string const & str);

	/// in pixels from top of screen
	void setScreenPos(LCursor & cur, int x, int y);
	/// in pixels from top of screen
	void getScreenPos(LCursor & cur, int & x, int & y);
	/// in pixels from left of screen
	int targetX(LCursor & cur);
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(LCursor & cur, idx_type & idx);
	/// go up to enclosing grid
	void popToEnclosingGrid(LCursor & cur);
	/// go up to the hull inset
	void popToEnclosingHull(LCursor & cur);
	/// go up to the hull inset
	void popToHere(LCursor & cur, MathInset const * p);
	/// adjust anchor position after deletions/insertions
	void adjust(LCursor & cur, pos_type from, difference_type diff);
	///
	InsetFormulaBase * formula();
	/// current offset in the current cell
	///
	bool script(LCursor & cur, bool);
	///
	bool interpret(LCursor & cur, char);
	/// interpret name a name of a macro
	void macroModeClose(LCursor & cur);
	/// are we currently typing the name of a macro?
	bool inMacroMode(LCursor & cur);
	/// get access to the macro we are currently typing
	MathUnknownInset * activeMacro(LCursor & cur);
	/// are we currently typing '#1' or '#2' or...?
	bool inMacroArgMode(LCursor & cur);
	/// are we in math mode (1), text mode (-1) or unsure?
	MathInset::mode_type currentMode(LCursor & cur);

	// Local selection methods
	///
	void selCopy(LCursor & cur);
	///
	void selCut(LCursor & cur);
	///
	void selDel(LCursor & cur);
	/// pastes n-th element of cut buffer
	void selPaste(LCursor & cur, size_t n);
	///
	void selHandle(LCursor & cur, bool);
	///
	void selStart(LCursor & cur);
	///
	void selClear(LCursor & cur);
	/// clears or deletes selection depending on lyxrc setting
	void selClearOrDel(LCursor & cur);
	/// draws light-blue selection background
	void drawSelection(PainterInfo & pi);
	/// replace selected stuff with at, placing the former
	// selection in given cell of atom
	void handleNest(LCursor & cur, MathAtom const & at, int cell = 0);
	/// remove this as soon as LyXFunc::getStatus is "localized"
	inline std::string getLastCode() { return "mathnormal"; }
	///
	bool isInside(MathInset const *);
	///
	char valign(LCursor & cur);
	///
	char halign(LCursor & cur);

	/// make sure cursor position is valid
	void normalize(LCursor & cur);
	/// mark current cursor trace for redraw
	void touch();

	/// enter a MathInset
	void push(LCursor & cur, MathAtom & par);
	/// enter a MathInset from the front
	void pushLeft(LCursor & cur, MathAtom & par);
	/// enter a MathInset from the back
	void pushRight(LCursor & cur, MathAtom & par);
	/// leave current MathInset to the left
	bool popLeft(LCursor & cur);
	/// leave current MathInset to the left
	bool popRight(LCursor & cur);

	/// returns the normalized anchor of the selection
	CursorSlice normalAnchor(LCursor & cur);

	/// dump selection information for debugging
	void seldump(char const * str);
	/// dump selection information for debugging
	void dump(char const * str);
	/// moves on
	void setSelection(LCursor & cur, CursorBase const & where, size_type n);
	/// grab selection marked by anchor and current cursor
	std::string grabSelection(LCursor & cur);
	/// guess what
	std::string grabAndEraseSelection(LCursor & cur);
	///
	void insert(LCursor & cur, char c);
	///
	void insert(LCursor & cur, std::string const & str);
	/// lock/unlock inset
	void insetToggle(LCursor & cur);

	/// hack for reveal codes
	void markInsert(LCursor & cur);
	void markErase(LCursor & cur);
	/// injects content of a cell into parent
	void pullArg(LCursor & cur);
	/// split font inset etc
	void handleFont(LCursor & cur, std::string const & font);
	///
	DispatchResult dispatch(LCursor & cur, FuncRequest const & cmd);

	/// pointer to enclsing LyX inset
	extern InsetFormulaBase * formula_;
}

void releaseMathCursor(LCursor & cur);

bool inMathed();

#endif
