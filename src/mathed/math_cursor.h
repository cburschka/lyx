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

#include "cursor_slice.h"
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


class MathCursor {
public:
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
	explicit MathCursor(BufferView *, InsetFormulaBase *, bool left);
	///
	~MathCursor();
	///
	void insert(BufferView & bv, MathAtom const &);
	///
	void insert(BufferView & bv, MathArray const &);
	///
	void insert2(BufferView & bv, std::string const &);
	///
	void paste(BufferView & bv, std::string const & data);
	/// return false for empty math insets
	bool erase(BufferView & bv);
	/// return false for empty math insets
	bool backspace(BufferView & bv);
	/// called for LFUN_HOME etc
	bool home(BufferView & bv, bool sel = false);
	/// called for LFUN_END etc
	bool end(BufferView & bv, bool sel = false);
	/// called for LFUN_RIGHT and LFUN_RIGHTSEL
	bool right(BufferView & bv, bool sel = false);
	/// called for LFUN_LEFT etc
	bool left(BufferView & bv, bool sel = false);
	/// called for LFUN_UP etc
	bool up(BufferView & bv, bool sel = false);
	/// called for LFUN_DOWN etc
	bool down(BufferView & bv, bool sel = false);
	/// Put the cursor in the first position
	void first(BufferView & bv);
	/// Put the cursor in the last position
	void last(BufferView & bv);
	/// move to next cell in current inset
	void idxNext(BufferView & bv);
	/// move to previous cell in current inset
	void idxPrev(BufferView & bv);
	///
	void plainErase(BufferView & bv);
	///
	void plainInsert(BufferView & bv, MathAtom const & at);
	///
	void niceInsert(BufferView & bv, MathAtom const & at);
	///
	void niceInsert(BufferView & bv, std::string const & str);

	/// in pixels from top of screen
	void setScreenPos(BufferView & bv, int x, int y);
	/// in pixels from top of screen
	void getScreenPos(BufferView & bv, int & x, int & y) const;
	/// in pixels from left of screen
	int targetX(BufferView & bv) const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(BufferView & bv, idx_type & idx) const;
	/// go up to enclosing grid
	void popToEnclosingGrid(BufferView & bv);
	/// go up to the hull inset
	void popToEnclosingHull(BufferView & bv);
	/// go up to the hull inset
	void popToHere(BufferView & bv, MathInset const * p);
	/// adjust anchor position after deletions/insertions
	void adjust(BufferView & bv, pos_type from, difference_type diff);
	///
	InsetFormulaBase * formula() const;
	/// current offset in the current cell
	///
	bool script(BufferView & bv, bool);
	///
	bool interpret(BufferView & bv, char);
	/// interpret name a name of a macro
	void macroModeClose(BufferView & bv);
	/// are we currently typing the name of a macro?
	bool inMacroMode(BufferView & bv) const;
	/// get access to the macro we are currently typing
	MathUnknownInset * activeMacro(BufferView & bv);
	/// get access to the macro we are currently typing
	MathUnknownInset const * activeMacro(BufferView & bv) const;
	/// are we currently typing '#1' or '#2' or...?
	bool inMacroArgMode(BufferView & bv) const;
	/// are we in math mode (1), text mode (-1) or unsure?
	MathInset::mode_type currentMode(BufferView & bv) const;

	// Local selection methods
	///
	bool selection() const;
	///
	void selCopy(BufferView & bv);
	///
	void selCut(BufferView & bv);
	///
	void selDel(BufferView & bv);
	/// pastes n-th element of cut buffer
	void selPaste(BufferView & bv, size_t n);
	///
	void selHandle(BufferView & bv, bool);
	///
	void selStart(BufferView & bv);
	///
	void selClear(BufferView & bv);
	/// clears or deletes selection depending on lyxrc setting
	void selClearOrDel(BufferView & bv);
	/// draws light-blue selection background
	void drawSelection(PainterInfo & pi) const;
	/// replace selected stuff with at, placing the former
	// selection in given cell of atom
	void handleNest(BufferView & bv, MathAtom const & at, int cell = 0);
	/// remove this as soon as LyXFunc::getStatus is "localized"
	std::string getLastCode() const { return "mathnormal"; }
	///
	bool isInside(MathInset const *) const;
	///
	char valign(BufferView & bv) const;
	///
	char halign(BufferView & bv) const;

	/// make sure cursor position is valid
	void normalize(BufferView & bv);
	/// mark current cursor trace for redraw
	void touch();

	/// enter a MathInset
	void push(BufferView & bv, MathAtom & par);
	/// enter a MathInset from the front
	void pushLeft(BufferView & bv, MathAtom & par);
	/// enter a MathInset from the back
	void pushRight(BufferView & bv, MathAtom & par);
	/// leave current MathInset to the left
	bool popLeft(BufferView & bv);
	/// leave current MathInset to the left
	bool popRight(BufferView & bv);

	///
	bool hasPrevAtom(BufferView & bv) const;
	///
	bool hasNextAtom(BufferView & bv) const;
	///
	MathAtom const & prevAtom(BufferView & bv) const;
	///
	MathAtom & prevAtom(BufferView & bv);
	///
	MathAtom const & nextAtom(BufferView & bv) const;
	///
	MathAtom & nextAtom(BufferView & bv);

	/// returns the selection
	void getSelection(BufferView & bv, CursorSlice &, CursorSlice &) const;
	/// returns the normalized anchor of the selection
	CursorSlice normalAnchor(BufferView & bv) const;

	/// how deep are we nested?
	unsigned depth(BufferView & bv) const;
	/// describe the situation
	std::string info(BufferView & bv) const;
	/// dump selection information for debugging
	void seldump(char const * str) const;
	/// dump selection information for debugging
	void dump(char const * str) const;
	/// moves on
	void setSelection(BufferView & bv, CursorBase const & where, size_type n);
	/// grab selection marked by anchor and current cursor
	std::string grabSelection(BufferView & bv) const;
	/// guess what
	std::string grabAndEraseSelection(BufferView & bv);
	///
	void insert(BufferView & bv, char c);
	///
	void insert(BufferView & bv, std::string const & str);
	/// lock/unlock inset
	void insetToggle(BufferView & bv);

	/// hack for reveal codes
	void markInsert(BufferView & bv);
	void markErase(BufferView & bv);
	/// injects content of a cell into parent
	void pullArg(BufferView & bv);
	/// split font inset etc
	void handleFont(BufferView & bv, std::string const & font);
	///
	DispatchResult dispatch(BufferView & bv, FuncRequest const & cmd);
private:
	/// moves cursor index one cell to the left
	bool idxLeft(BufferView & bv);
	/// moves cursor index one cell to the right
	bool idxRight(BufferView & bv);
	/// moves cursor to end of last cell of current line
	bool idxLineLast(BufferView & bv);
	/// moves cursor position one cell to the left
	bool posLeft(BufferView & bv);
	/// moves cursor position one cell to the right
	bool posRight(BufferView & bv);
	/// moves position somehow up or down
	bool goUpDown(BufferView & bv, bool up);
	/// moves position closest to (x, y) in given box
	bool bruteFind(BufferView & bv,
		int x, int y, int xlow, int xhigh, int ylow, int yhigh);
	/// moves position closest to (x, y) in current cell
	void bruteFind2(BufferView & bv, int x, int y);
	/// are we in a nucleus of a script inset?
	bool inNucleus(BufferView & bv) const;

	/// erase the selected part and re-sets the cursor
	void eraseSelection(BufferView & bv);

	/// the name of the macro we are currently inputting
	std::string macroName(BufferView & bv) const;
	/// where in the curent cell does the macro name start?
	difference_type macroNamePos(BufferView & bv) const;
	/// can we enter the inset?
	bool openable(MathAtom const &, bool selection) const;

	/// pointer to enclsing LyX inset
	InsetFormulaBase * formula_;
	// Selection stuff
	/// text code of last char entered
	//MathTextCodes lastcode_;
	/// do we allow autocorrection
	bool autocorrect_;
	/// do we currently select
	bool selection_;
	/// are we entering a macro name?
	bool macromode_;
};

extern MathCursor * mathcursor;
void releaseMathCursor(BufferView & bv);

#endif
