// -*- C++ -*-
/**
 *  File:        math_cursor.h
 *  Purpose:     Declaration of interaction classes for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1996
 *  Description: MathCursor control all user interaction
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_CURSOR
#define MATH_CURSOR

#ifdef __GNUG__
#pragma interface
#endif

#include "math_inset.h"
#include "math_iterator.h"
#include "LString.h"

class InsetFormulaBase;
class BufferView;
class MathPainterInfo;
class MathUnknownInset;
class Selection;

/**

[Have a look at math_inset.h first]

The MathCursor is different from the kind of cursor used in the Outer
World. It contains a stack of MathCursorPos, each of which is made
up of a inset pointer, an index and a position offset, marking a path from
this formula's MathHullInset to the current position.

*/


class MathCursor {
public:
	/// short of anything else reasonable
	typedef MathInset::size_type       size_type;
	/// type for cursor positions within a cell
	typedef MathInset::pos_type        pos_type;
	/// type for cell indices
	typedef MathInset::idx_type        idx_type;
	/// type for row numbers
	typedef MathInset::row_type        row_type;
	/// type for column numbers
	typedef MathInset::col_type        col_type;

	///
	explicit MathCursor(InsetFormulaBase *, bool left);
	///
	~MathCursor();
	///
	void insert(MathAtom const &);
	///
	void insert(MathArray const &);
	///
	void paste(string const & data);
	///
	void erase();
	///
	void backspace();
	/// called for LFUN_HOME etc
	bool home(bool sel = false);
	/// called for LFUN_END etc
	bool end(bool sel = false);
	/// called for LFUN_RIGHT and LFUN_RIGHTSEL
	bool right(bool sel = false);
	/// called for LFUN_LEFT etc
	bool left(bool sel = false);
	/// called for LFUN_UP etc
	bool up(bool sel = false);
	/// called for LFUN_DOWN etc
	bool down(bool sel = false);
	/// Put the cursor in the first position
	void first();
	/// Put the cursor in the last position
	void last();
	/// move to next cell in current inset
	void idxNext();
	/// move to previous cell in current inset
	void idxPrev();
	///
	void plainErase();
	///
	void plainInsert(MathAtom const &);
	///
	void niceInsert(MathAtom const &);
	///
	void niceInsert(string const &);

	/// in pixels from top of screen
	void setPos(int x, int y);
	/// in pixels from top of screen
	void getPos(int & x, int & y) const;
	/// in pixels from left of screen
	int targetX() const;
	/// current inset
	MathInset * par() const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(idx_type & idx) const;
	/// go up to enclosing grid
	void popToEnclosingGrid();
	/// go up to the hull inset
	void popToEnclosingHull();
	/// go up to the hull inset
	void popToHere(MathInset const * p);
	///
	InsetFormulaBase * formula() const;
	/// current offset in the current cell
	pos_type pos() const;
	/// current cell
	idx_type idx() const;
	/// size of current cell
	size_type size() const;
	///
	bool script(bool);
	/// 
	bool interpret(char);
	/// interpret name a name of a macro
	void macroModeClose();
	/// are we currently typing the name of a macro?
	bool inMacroMode() const;
	/// get access to the macro we are currently typing
	MathUnknownInset * activeMacro();
	/// get access to the macro we are currently typing
	MathUnknownInset const * activeMacro() const;
	/// are we currently typing '#1' or '#2' or...?
	bool inMacroArgMode() const;
	/// are we in math mode (1), text mode (-1) or unsure?
	MathInset::mode_type currentMode() const;

	// Local selection methods
	///
	bool selection() const;
	///
	void selCopy();
	///
	void selCut();
	///
	void selDel();
	///
	void selPaste();
	///
	void selHandle(bool);
	///
	void selStart();
	///
	void selClear();
	/// clears or deletes selection depending on lyxrc setting
	void selClearOrDel();
	/// draws light-blue selection background
	void drawSelection(MathPainterInfo & pi) const;
	///
	void handleNest(MathAtom const & at);
	/// remove this as soon as LyXFunc::getStatus is "localized"
	string getLastCode() const { return "mathnormal"; }
	///
	bool isInside(MathInset const *) const;
	///
	char valign() const;
	///
	char halign() const;

	/// make sure cursor position is valid
	void normalize();
	/// mark current cursor trace for redraw
	void touch();
	///
	UpdatableInset * asHyperActiveInset() const;

	/// enter a MathInset
	void push(MathAtom & par);
	/// enter a MathInset from the front
	void pushLeft(MathAtom & par);
	/// enter a MathInset from the back
	void pushRight(MathAtom & par);
	/// leave current MathInset to the left
	bool popLeft();
	/// leave current MathInset to the left
	bool popRight();

	///
	MathArray & array() const;
	///
	bool hasPrevAtom() const;
	///
	bool hasNextAtom() const;
	///
	MathAtom const & prevAtom() const;
	///
	MathAtom & prevAtom();
	///
	MathAtom const & nextAtom() const;
	///
	MathAtom & nextAtom();

	/// returns the selection
	void getSelection(MathCursorPos &, MathCursorPos &) const;
	/// returns the normalized anchor of the selection
	MathCursorPos normalAnchor() const;

	/// reference to the last item of the path, i.e. "The Cursor"
	MathCursorPos & cursor();
	/// reference to the last item of the path, i.e. "The Cursor"
	MathCursorPos const & cursor() const;
	/// how deep are we nested?
	unsigned depth() const;

	/// local dispatcher
	MathInset::result_type dispatch(FuncRequest const & cmd);
	/// describe the situation
	string info() const;
	/// dump selection information for debugging
	void seldump(char const * str) const;
	/// dump selection information for debugging
	void dump(char const * str) const;
	/// moves on
	void setSelection(MathIterator const & where, size_type n);
	///
	void insert(char c);
	///
	void insert(string const & str);
	/// lock/unlock inset
	void insetToggle();

	/// hack for reveal codes
	void markInsert();
	void markErase();
	//void handleExtern(string const & arg);

	///
	friend class Selection;


private:
	/// injects content of a cell into parent
	void pullArg();
	/// moves cursor index one cell to the left
	bool idxLeft();
	/// moves cursor index one cell to the right
	bool idxRight();
	/// moves cursor to end of last cell of current line
	bool idxLineLast();
	/// moves cursor position one cell to the left
	bool posLeft();
	/// moves cursor position one cell to the right
	bool posRight();
	/// moves position somehow up or down
	bool goUpDown(bool up);
	/// moves position closest to (x, y) in given box
	bool bruteFind(int x, int y, int xlow, int xhigh, int ylow, int yhigh);
	/// moves position closest to (x, y) in current cell
	void bruteFind2(int x, int y);
	/// are we in a nucleus of a script inset?
	bool inNucleus() const;

	/// grab selection marked by anchor and current cursor 
	string grabSelection() const;
	/// erase the selected part and re-sets the cursor
	void eraseSelection();
	/// guess what
	string grabAndEraseSelection();

	/// the name of the macro we are currently inputting
	string macroName() const;
	/// where in the curent cell does the macro name start?
	MathInset::difference_type macroNamePos() const;
	/// can we enter the inset?
	bool openable(MathAtom const &, bool selection) const;
	/// write access to cursor cell position
	pos_type & pos();
	/// write access to cursor cell index
	idx_type & idx();

	/// path of positions the cursor had to go if it were leaving each inset
	MathIterator Cursor_;
	/// path of positions the anchor had to go if it were leaving each inset
	mutable MathIterator Anchor_;
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
	/// are we targeting a certain x coordinate, if so, which one?
	int targetx_;
};

extern MathCursor * mathcursor;
void releaseMathCursor(BufferView * bv);

#endif
