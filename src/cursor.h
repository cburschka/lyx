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

#include "dispatchresult.h"
#include "dociterator.h"

#include <iosfwd>
#include <vector>

class Buffer;
class BufferView;
class FuncStatus;
class FuncRequest;
class Point;
class LyXFont;

// these should go
class MathUnknownInset;
class Encoding;


/// The cursor class describes the position of a cursor within a document.

// The public inheritance should go in favour of a suitable data member
// (or maybe private inheritance) at some point of time.
class LCursor : public DocIterator {
public:
	/// create the cursor of a BufferView
	explicit LCursor(BufferView & bv);

	/// dispatch from innermost inset upwards
	void dispatch(FuncRequest const & cmd);
	/// get the resut of the last dispatch
	DispatchResult result() const;
	/// add a new cursor slice
	void push(InsetBase & inset);
	/// add a new cursor slice, place cursor on left end
	void pushLeft(InsetBase & inset);
	/// pop one level off the cursor
	void pop();
	/// pop one slice off the cursor stack and go left
	bool popLeft();
	/// pop one slice off the cursor stack and go right
	bool popRight();
	/// make sure cursor is outside given inset
	void leaveInset(InsetBase const & inset);
	/// sets cursor part
	void setCursor(DocIterator const & it);

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
	void setSelection(DocIterator const & where, size_t n);
	///
	void clearSelection();
	/// access start of selection
	CursorSlice selBegin() const;
	/// access end of selection
	CursorSlice selEnd() const;
	/// access start of selection
	DocIterator selectionBegin() const;
	/// access start of selection
	DocIterator selectionEnd() const;
	///
	void selHandle(bool selecting);
	//
	std::string selectionAsString(bool label) const;
	///
	void paste(std::string const & data);
	///
	std::string currentState();

	/// auto-correct mode
	bool autocorrect() const { return autocorrect_; }
	/// auto-correct mode
	bool & autocorrect() { return autocorrect_; }
	/// are we entering a macro name?
	bool macromode() const { return macromode_; }
	/// are we entering a macro name?
	bool & macromode() { return macromode_; }
	/// returns x,y position
	void getPos(int & x, int & y) const;

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

	/// in pixels from left of screen
	int targetX() const;
	/// write acess to target x position of cursor
	int & x_target();
	/// return target x position of cursor
	int x_target() const;
	/// set targetX in current position
	void setTargetX();
	/// clear target x position of cursor
	void clearTargetX();

	/// access to normalized selection anchor
	CursorSlice anchor() const;
	/// sets anchor to cursor position
	void resetAnchor();
	/// access to owning BufferView
	BufferView & bv() const;
	/// access to owning Buffer
	Buffer & buffer() const;
	/// get some interesting description of top position
	void info(std::ostream & os) const;
	/// are we in math mode (2), text mode (1) or unsure (0)?
	int currentMode();
	/// reset cursor bottom to the beginning of the given inset
	// (sort of 'chroot' environment...)
	void reset(InsetBase &);
	/// for spellchecking
	void replaceWord(std::string const & replacestring);
	/// the event was not (yet) dispatched
	void undispatched();
	/// the event was already dispatched
	void dispatched();
	/// call update() when done
	void needsUpdate();
	/// don't call update() when done
	void noUpdate();
	/// fix cursor in circumstances that should never happen
	void fixIfBroken();

	/// output
	friend std::ostream & operator<<(std::ostream & os, LCursor const & cur);

public:
	///
	BufferView * bv_;
//private:
	/// the anchor position
	DocIterator anchor_;

	///
	DispatchResult disp_;


private:
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
	/// do we have a selection?
	bool selection_;
	/// are we on the way to get one?
	bool mark_;

private:

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
	/// current offset in the top cell
	/// interpret name a name of a macro
	void macroModeClose();
	/// are we currently typing the name of a macro?
	bool inMacroMode() const;
	/// get access to the macro we are currently typing
	MathUnknownInset * activeMacro();

	/// replace selected stuff with at, placing the former
	// selection in given cell of atom
	void handleNest(MathAtom const & at, int cell = 0);
	///
	bool isInside(InsetBase const *);

	/// make sure cursor position is valid
	void normalize();
	/// mark current cursor trace for redraw
	void touch();

	/// hack for reveal codes
	void markInsert();
	void markErase();
	/// injects content of a cell into parent
	void pullArg();
	/// split font inset etc
	void handleFont(std::string const & font);

	/// display a message
	void message(std::string const & msg) const;
	/// display an error message
	void errorMessage(std::string const & msg) const;
	///
	std::string getPossibleLabel();

	/// moves position somehow up or down
	bool goUpDown(bool up);

	/// the name of the macro we are currently inputting
	std::string macroName();
	/// where in the curent cell does the macro name start?
	int macroNamePos();
	/// can we enter the inset?
	bool openable(MathAtom const &) const;
	///
	Encoding const * getEncoding() const;
	/// font at cursor position
	LyXFont getFont() const;
};


#endif // LYXCURSOR_H
