// -*- C++ -*-
/**
 * \file cursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CURSOR_H
#define CURSOR_H

#include "cursor_slice.h"

#include <iosfwd>
#include <vector>

class BufferView;
class UpdatableInset;
class MathAtom;
class DispatchResult;
class FuncRequest;
class InsetTabular;
class LyXText;
class Paragraph;
class Row;


// these should go
class MathHullInset;
class MathUnknownInset;
class MathGridInset;


// only needed for gcc 2.95, remove when support terminated
template <typename A, typename B>
bool ptr_cmp(A const * a, B const * b)
{
	return a == b;
}


// this is used for traversing math insets
typedef std::vector<CursorSlice> CursorBase;
/// move on one step
void increment(CursorBase &);
///
CursorBase ibegin(InsetBase * p);
///
CursorBase iend(InsetBase * p);


/**
 * The cursor class describes the position of a cursor within a document.
 */

class LCursor {
public:
	/// type for cell number in inset
	typedef CursorSlice::idx_type idx_type;
	/// type for paragraph numbers positions within a cell
	typedef CursorSlice::par_type par_type;
	/// type for cursor positions within a cell
	typedef CursorSlice::pos_type pos_type;
	/// type for row indices
	typedef CursorSlice::row_type row_type;
	/// type for col indices
	typedef CursorSlice::col_type col_type;

	/// create the cursor of a BufferView
	explicit LCursor(BufferView & bv);
	/// dispatch from innermost inset upwards
	DispatchResult dispatch(FuncRequest const & cmd);
	/// add a new cursor slice
	void push(InsetBase * inset);
	/// add a new cursor slice, place cursor on left end
	void pushLeft(InsetBase * inset);
	/// pop one level off the cursor
	void pop();
	/// pop one slice off the cursor stack and go left
	bool popLeft();
	/// pop one slice off the cursor stack and go right
	bool popRight();
	/// restrict cursor nesting to given size
	void pop(int depth);
	/// access to current cursor slice
	CursorSlice & current();
	/// access to current cursor slice
	CursorSlice const & current() const;
	/// how many nested insets do we have?
	size_t depth() const { return cursor_.size(); }
	/// depth of current slice
	int currentDepth() const { return current_; }

	//
	// selection
	//
	/// selection active?
	bool selection() const { return selection_; }
	/// selection active?
	bool & selection() { return selection_; }
	/// did we place the anchor?
	bool mark() const { return mark_; }
	/// did we place the anchor?
	bool & mark() { return mark_; }
	///
	void setSelection();
	/// set selection at given position
	void setSelection(CursorBase const & where, size_t n);
	///
	void clearSelection();
	/// access start of selection
	CursorSlice & selBegin();
	/// access start of selection
	CursorSlice const & selBegin() const;
	/// access end of selection
	CursorSlice & selEnd();
	/// access end of selection
	CursorSlice const & selEnd() const;
	///
	std::string grabSelection();
	///
	void eraseSelection();
	///
	std::string grabAndEraseSelection();
	// other selection methods
	///
	void selCopy();
	///
	void selCut();
	///
	void selDel();
	/// pastes n-th element of cut buffer
	void selPaste(size_t n);
	///
	void selHandle(bool selecting);
	/// start selection
	void selStart();
	/// clear selection
	void selClear();
	/// clears or deletes selection depending on lyxrc setting
	void selClearOrDel();
	//
	std::string selectionAsString(bool label) const;
	///
	void paste(std::string const & data);
	///
	std::string currentState();

	//
	// access to the 'current' cursor slice
	//
	/// the containing inset
	InsetBase * inset() const { return current().inset(); }
	/// return the cell of the inset this cursor is in
	idx_type idx() const { return current().idx(); }
	/// return the cell of the inset this cursor is in
	idx_type & idx() { return current().idx(); }
	/// return the last possible cell in this inset
	idx_type lastidx() const { return current().lastidx(); }
	/// return the paragraph this cursor is in
	par_type par() const { return current().par(); }
	/// return the paragraph this cursor is in
	par_type & par() { return current().par(); }
	/// return the last possible paragraph in this inset
	par_type lastpar() const;
	/// return the position within the paragraph
	pos_type pos() const { return current().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return current().pos(); }
	/// return the last position within the paragraph
	pos_type lastpos() const;
	/// return the display row of the cursor with in the current par
	row_type crow() const;
	/// return the display row of the cursor with in the current par
	row_type lastcrow() const;

	/// return the number of embedded cells
	size_t nargs() const;
	/// return the number of embedded cells
	size_t ncols() const;
	/// return the number of embedded cells
	size_t nrows() const;
	/// return the grid row of the current cell
	row_type row() const;
	/// return the grid row of the current cell
	col_type col() const;
	/// the inset just behind the cursor
	InsetBase * nextInset();
	/// the inset just in front of the cursor
	InsetBase * prevInset();
	/// the inset just in front of the cursor
	InsetBase const * prevInset() const;

	//
	// math-specific part
	//
	/// return the mathed cell this cursor is in
	MathArray const & cell() const;
	/// return the mathed cell this cursor is in
	MathArray & cell();
	/// the mathatom left of the cursor
	MathAtom const & prevAtom() const;
	/// the mathatom left of the cursor
	MathAtom & prevAtom();
	/// the mathatom right of the cursor
	MathAtom const & nextAtom() const;
	/// the mathatom right of the cursor
	MathAtom & nextAtom();
	/// auto-correct mode
	bool autocorrect() const { return autocorrect_; }
	/// auto-correct mode
	bool & autocorrect() { return autocorrect_; }
	/// are we entering a macro name?
	bool macromode() const { return macromode_; }
	/// are we entering a macro name?
	bool & macromode() { return macromode_; }

	//
	// text-specific part
	/// see comment for boundary_ below
	bool boundary() const { return current().boundary(); }
	/// see comment for boundary_ below
	bool & boundary() { return current().boundary(); }
	/// the paragraph we're in
	Paragraph & paragraph();
	/// the paragraph we're in
	Paragraph const & paragraph() const;
	/// the row in the paragraph we're in
	Row & textRow();
	/// the row in the paragraph we're in
	Row const & textRow() const;
	///
	LyXText * text() const;
	///
	InsetBase * innerInsetOfType(int code) const;
	///
	InsetTabular * innerInsetTabular() const;
	///
	LyXText * innerText() const;
	///
	CursorSlice const & innerTextSlice() const;
	/// returns x,y position
	void getPos(int & x, int & y) const;
	/// returns cursor dimension
	void getDim(int & asc, int & desc) const;

	//
	// common part
	//
	/// move one step to the left
	bool posLeft();
	/// move one step to the right
	bool posRight();

	/// insert an inset
	void insert(InsetBase *);
	/// insert a single char
	void insert(char c);
	/// insert a string
	void insert(std::string const & str);

	/// write acess to target x position of cursor
	int & x_target();
	/// return target x position of cursor
	int x_target() const;
	/// clear target x position of cursor
	void clearTargetX();

	/// access to selection anchor
	CursorSlice & anchor();
	/// access to selection anchor
	CursorSlice const & anchor() const;
	/// cache the absolute coordinate from the top inset
	void updatePos();
	/// sets anchor to cursor position
	void resetAnchor(); 
	/// access to owning BufferView
	BufferView & bv() const; 
	/// get some interesting description of current position
	void info(std::ostream & os) const;
	/// are we in math mode (2), text mode (1) or unsure (0)?
	int currentMode();
	/// reset cursor
	void reset();
	/// for spellchecking
	void replaceWord(std::string const & replacestring);

	/// output
	friend std::ostream & operator<<(std::ostream & os, LCursor const & cur);
public:
//private:
	/// mainly used as stack, but wee need random access
	std::vector<CursorSlice> cursor_;
	/// the anchor position
	std::vector<CursorSlice> anchor_;

private:
	///
	BufferView * bv_;
	/// current slice
	int current_;
	///
	int cached_y_;
	/**
	 * The target x position of the cursor. This is used for when
	 * we have text like :
	 *
	 * blah blah blah blah| blah blah blah
	 * blah blah blah
	 * blah blah blah blah blah blah
	 *
	 * When we move onto row 3, we would like to be vertically aligned
	 * with where we were in row 1, despite the fact that row 2 is
	 * shorter than x()
	 */
	int x_target_;
	// do we have a selection?
	bool selection_;
	// are we on the way to get one?
	bool mark_;

	//
	// math specific stuff that could be promoted to "global" later
	//
	/// do we allow autocorrection
	bool autocorrect_;
	/// are we entering a macro name?
	bool macromode_;


///////////////////////////////////////////////////////////////////
//
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to the math insets.
//
///////////////////////////////////////////////////////////////////

public:
	///
	void insert(MathAtom const &);
	///
	void insert(MathArray const &);
	/// return false for empty math insets
	bool erase();
	/// return false for empty math insets
	bool backspace();
	/// called for LFUN_HOME etc
	bool home();
	/// called for LFUN_END etc
	bool end();
	/// called for LFUN_RIGHT and LFUN_RIGHTSEL
	bool right();
	/// called for LFUN_LEFT etc
	bool left();
	/// called for LFUN_UP etc
	bool up();
	/// called for LFUN_DOWN etc
	bool down();
	///
	void plainErase();
	///
	void plainInsert(MathAtom const & at);
	///
	void niceInsert(MathAtom const & at);
	///
	void niceInsert(std::string const & str);

	/// in pixels from top of screen
	void setScreenPos(int x, int y);
	/// in pixels from left of screen
	int targetX() const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(idx_type & idx) const;
	/// adjust anchor position after deletions/insertions
	void adjust(pos_type from, int diff);
	///
	MathHullInset * formula() const;
	/// current offset in the current cell
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
	/// are we currently typing '#1' or '#2' or...?
	bool inMacroArgMode() const;

	/// replace selected stuff with at, placing the former
	// selection in given cell of atom
	void handleNest(MathAtom const & at, int cell = 0);
	/// remove this as soon as LyXFunc::getStatus is "localized"
	//inline std::string getLastCode() { return "mathnormal"; }
	///
	bool isInside(InsetBase const *);
	///
	char valign();
	///
	char halign();

	/// make sure cursor position is valid
	void normalize();
	/// mark current cursor trace for redraw
	void touch();

	/// returns the normalized anchor of the selection
	CursorSlice normalAnchor();

	/// lock/unlock inset
	void lockToggle();

	/// hack for reveal codes
	void markInsert();
	void markErase();
	/// injects content of a cell into parent
	void pullArg();
	/// split font inset etc
	void handleFont(std::string const & font);

	void releaseMathCursor();
	/// are we in mathed?
	bool inMathed() const;
	/// are we in texted?
	bool inTexted() const;

	/// display a message
	void message(std::string const & msg) const;
	/// display an error message
	void errorMessage(std::string const & msg) const;

private:
	/// moves cursor index one cell to the left
	bool idxLeft();
	/// moves cursor index one cell to the right
	bool idxRight();
	/// moves cursor to end of last cell of current line
	bool idxLineLast();
	/// moves position somehow up or down
	bool goUpDown(bool up);
	/// moves position closest to (x, y) in given box
	bool bruteFind(int x, int y, int xlow, int xhigh, int ylow, int yhigh);
	/// moves position closest to (x, y) in current cell
	void bruteFind2(int x, int y);
	/// are we in a nucleus of a script inset?
	bool inNucleus();

	/// the name of the macro we are currently inputting
	std::string macroName();
	/// where in the curent cell does the macro name start?
	int macroNamePos();
	/// can we enter the inset?
	bool openable(MathAtom const &);
};

#endif // LYXCURSOR_H
