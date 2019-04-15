// -*- C++ -*-
/**
 * \file Cursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

/*
First some explanation about what a Cursor really is, from local to
global.

* a CursorSlice indicates the position of the cursor at local level.
  It contains in particular:
  * idx(): the cell that contains the cursor (for Tabular or math
           arrays). Always 0 for 'plain' insets
  * pit(): the index of the current paragraph (only for text)
  * pos(): the position in the current paragraph (or in the math
           equation in mathed).
  * inset(): the inset in which the cursor is. For a InsetTabular,
    this is the tabular itself, not the cell inset (which is an
    InsetTableCell).

* a DocIterator indicated the position of the cursor in the document.
  It knows about the current buffer (buffer() method) and contains a
  vector of CursorSlices that describes the nesting of insets up to the
  point of interest. Note that operator<< has been implemented, so that
  one can send a DocIterator to a stream to see its value. Try it, it is
  very helpful to understand the cursor layout.
  * when using idx/pit/pos on a DocIterator, one gets the information
    from the inner slice (this slice can be accessed as top())
  * inMathed() returns true when the cursor is in a math formula
  * inTexted() returns true when the cursor is in text
  * innerTextSlice() returns the deepest slice that is text (useful
    when one is in a math equation and looks for the enclosing text)

* A CursorData is a descendant of Dociterator that contains
  * a second DocIterator object, the anchor, that is useful when
    selecting.
  * some other data that describes current selection, cursor font, etc.

  This class is mostly used only for undo and contains the Cursor
  elements that are not GUI-related. In LyX 2.0, Cursor was directly
  deriving from DocIterator

* A Cursor is a descendant of CursorData that contains interesting
  display-related information, in particular the BufferView that owns it.
*/

#ifndef LCURSOR_H
#define LCURSOR_H

#include "DispatchResult.h"
#include "DocIterator.h"
#include "Font.h"
#include "Undo.h"

#include "mathed/MathParser_flags.h"


namespace lyx {

class Buffer;
class BufferView;
class FuncStatus;
class FuncRequest;
class Row;

// these should go
class InsetMathUnknown;

/**
 * This class describes the position of a cursor within a document,
 * but does not contain any detail about the view. It is currently
 * only used to save cursor position in Undo, but culd be extended to
 * handle the methods that only need this data.
 **/
class CursorData : public DocIterator
{
public:
	///
	CursorData();
	///
	explicit CursorData(Buffer * buffer);
	///
	explicit CursorData(DocIterator const & dit);
	/// output
	friend std::ostream & operator<<(std::ostream & os, CursorData const & cur);
	friend LyXErr & operator<<(LyXErr & os, CursorData const & cur);

	/// reset cursor bottom to the beginning of the top inset
	// (sort of 'chroot' environment...)
	void reset();
	/// sets cursor part
	/// this (intentionally) does neither touch anchor nor selection status
	void setCursor(DocIterator const & it);
	/// set the cursor to dit normalised against the anchor, and set selection.
	void setCursorSelectionTo(DocIterator dit);
	/// sets the cursor to the normalized selection anchor
	void setCursorToAnchor();

	//
	// selection
	//
	/// selection active?
	bool selection() const { return selection_; }
	/// set selection; this is lower level than (set|clear)Selection
	void selection(bool sel) { selection_ = sel; }
	/// do we have a multicell selection?
	bool selIsMultiCell() const
		{ return selection_ && selBegin().idx() != selEnd().idx(); }
	/// do we have a multiline selection?
	bool selIsMultiLine() const
		{ return selection_ && selBegin().pit() != selEnd().pit(); }
	///
	void setWordSelection(bool set) { word_selection_ = set; }
	///
	bool wordSelection() { return word_selection_; }
	/// did we place the anchor?
	bool mark() const { return mark_; }
	/// did we place the anchor?
	void setMark(bool mark) { mark_ = mark; }
	///
	void setSelection();
	/// set selection at given position
	void setSelection(DocIterator const & where, int n);
	///
	void clearSelection();
	/// check whether selection contains specific type of inset
	/// returns 0 if no selection was made
	bool insetInSelection(InsetCode const & inset);
	/// count occurences of specific inset type in the selection
	/// returns 0 if no selection was made
	int countInsetsInSelection(InsetCode const & inset);

	/// access to normalized selection anchor
	CursorSlice normalAnchor() const;
	/// access to real selection anchor
	DocIterator const & realAnchor() const { return anchor_; }
	DocIterator & realAnchor() { return anchor_; }
	/// sets anchor to cursor position
	void resetAnchor();

	/// access start of selection
	CursorSlice selBegin() const;
	/// access end of selection
	CursorSlice selEnd() const;
	/// access start of selection
	DocIterator selectionBegin() const;
	/// access end of selection
	DocIterator selectionEnd() const;

	///
	docstring selectionAsString(bool with_label) const;
	/// get some interesting description of top position
	void info(odocstream & os, bool devel_mode) const;
	///
	docstring currentState(bool devel_mode) const;

	/// auto-correct mode
	bool autocorrect() const { return autocorrect_; }
	/// auto-correct mode
	bool & autocorrect() { return autocorrect_; }

	/// fix cursor in circumstances that should never happen.
	/// \retval true if a fix occurred.
	bool fixIfBroken();
	/// Repopulate the slices insets from bottom to top. Useful
	/// for stable iterators or Undo data.
	void sanitize();

	///
	bool undoAction();
	///
	bool redoAction();

	/// makes sure the next operation will be stored
	void finishUndo() const;
	/// open a new group of undo operations. Groups can be nested.
	void beginUndoGroup() const;
	/// end the current undo group
	void endUndoGroup() const;
	/// end abruptly the current group and create a new one wih the same nesting level
	void splitUndoGroup() const;

	/// The general case: prepare undo for an arbitrary range.
	void recordUndo(pit_type from, pit_type to) const;
	/// Convenience: prepare undo for the range between 'from' and cursor.
	void recordUndo(pit_type from) const;
	/// Convenience: prepare undo for the single paragraph or cell
	/// containing the cursor
	void recordUndo(UndoKind kind = ATOMIC_UNDO) const;
	/// Convenience: prepare undo for the inset containing the cursor
	void recordUndoInset(Inset const * inset = 0) const;
	/// Convenience: prepare undo for the whole buffer
	void recordUndoFullBuffer() const;
	/// Convenience: prepare undo for buffer parameters
	void recordUndoBufferParams() const;
	/// Convenience: prepare undo for the selected paragraphs or cells
	void recordUndoSelection() const;

	/// hook for text input to maintain the "new born word"
	void markNewWordPosition();
	/// The position of the new born word
	/// As the user is entering a word without leaving it
	/// the result is not empty. When not in text mode
	/// and after leaving the word the result is empty.
	DocIterator newWord() const { return new_word_; }

	/// are we in math mode (2), text mode (1) or unsure (0)?
	int currentMode();

	/// Return true if the next or previous inset has confirmDeletion depending
	/// on the boolean before. If there is a selection, return true if at least
	/// one inset in the selection has confirmDeletion.
	bool confirmDeletion(bool before = false) const;

private:
	/// validate the "new born word" position
	void checkNewWordPosition();
	/// clear the "new born word" position
	void clearNewWordPosition();

	/// the anchor position
	DocIterator anchor_;
	/// do we have a selection?
	bool selection_;
	/// are we on the way to get one?
	bool mark_;
	/// are we in word-selection mode? This is set when double clicking.
	bool word_selection_;

	/// the start of the new born word
	DocIterator new_word_;
	//
	// math specific stuff that could be promoted to "global" later
	//
	/// do we allow autocorrection
	bool autocorrect_;

	// FIXME: make them private
public:
	/// The current font settings. This holds the settings for output.
	Font current_font;
	/// The current display font. This holds the settings of the text
	/// in the workarea.
	Font real_current_font;
};


/// The cursor class describes the position of a cursor within a document.
class Cursor : public CursorData
{
public:
	/// create the cursor of a BufferView
	explicit Cursor(BufferView & bv);

	/// add a new cursor slice
	void push(Inset & inset);
	/// add a new cursor slice, place cursor at front (move backwards)
	void pushBackward(Inset & inset);
	/// try to put cursor in inset before it in entry cell, or next one
	/// if it is not empty, or exit the slice if there is no next one.
	void editInsertedInset();

	/// pop one level off the cursor
	void pop();
	/// pop one slice off the cursor stack and go backwards
	bool popBackward();
	/// pop one slice off the cursor stack and go forward
	bool popForward();
	/// set the cursor data
	void setCursorData(CursorData const & data);

	/// returns true if we made a decision
	bool getStatus(FuncRequest const & cmd, FuncStatus & flag) const;
	/// dispatch from innermost inset upwards
	void dispatch(FuncRequest const & cmd);
	/// display a message
	void message(docstring const & msg) const;
	/// display an error message
	void errorMessage(docstring const & msg) const;
	/// get the resut of the last dispatch
	DispatchResult const & result() const;

	///
	void setCurrentFont();

	/**
	 * Update the selection status and save permanent
	 * selection if needed.
	 * @param selecting the new selection status
	 * @return whether the selection status has changed
	 */
	bool selHandle(bool selecting);

	/// returns x,y position
	void getPos(int & x, int & y) const;
	/// return logical positions between which the cursor is situated
	/**
	 * If the cursor is at the edge of a row, the position which is "over the
	 * edge" will be returned as -1.
	 */
	void getSurroundingPos(pos_type & left_pos, pos_type & right_pos) const;
	/// the row in the paragraph we're in
	Row const & textRow() const;

	//
	// common part
	//
	/// move visually one step to the right
	/**
	 * @note: This method may move into an inset unless skip_inset == true.
	 * @note: This method may move into a new paragraph.
	 * @note: This method may move out of the current slice.
	 * @return: true if moved, false if not moved
	 */
	bool posVisRight(bool skip_inset = false);
	/// move visually one step to the left
	/**
	 * @note: This method may move into an inset unless skip_inset == true.
	 * @note: This method may move into a new paragraph.
	 * @note: This method may move out of the current slice.
	 * @return: true if moved, false if not moved
	 */
	bool posVisLeft(bool skip_inset = false);
	/// move visually to next/previous row
	/**
	 * Assuming we were to keep moving left (right) from the current cursor
	 * position, place the cursor at the rightmost (leftmost) edge of the
	 * new row to which we would move according to visual-mode cursor movement.
	 * This could be either the next or the previous row, depending on the
	 * direction in which we're moving, and whether we're in an LTR or RTL
	 * paragraph.
	 * @note: The new position may even be in a new paragraph.
	 * @note: This method will not move out of the current slice.
	 * @return: false if not moved (no more rows to move to in given direction)
	 * @return: true if moved
	 */
	bool posVisToNewRow(bool movingLeft);
	/// move to right or left extremity of the current row
	void posVisToRowExtremity(bool left);
	/// Should interpretation of the arrow keys be reversed?
	bool reverseDirectionNeeded() const;

	///
	///  Insertion (mathed and texted)
	///
	/// insert a single char
	void insert(char_type c);
	/// insert a string
	void insert(docstring const & str);
	/// insert an inset
	void insert(Inset *);
	///
	///  Insertion (mathed only)
	///
	/// insert a math atom
	void insert(MathAtom const &);
	/// insert a string of atoms
	void insert(MathData const &);
	/// Like insert, but moves the selection inside the inset if possible
	void niceInsert(MathAtom const & at);
	/// return the number of inserted array items
	/// FIXME: document properly
	int niceInsert(docstring const & str, Parse::flags f = Parse::NORMAL,
			bool enter = true);

	/// FIXME: rename to something sensible showing difference to x_target()
	/// in pixels from left of screen, set to current position if unset
	int targetX() const;
	/// set the targetX to x
	void setTargetX(int x);
	/// return targetX or -1 if unset
	int x_target() const;
	/// set targetX to current position
	void setTargetX();
	/// clear targetX, i.e. set it to -1
	void clearTargetX();
	/// set offset to actual position - targetX
	void updateTextTargetOffset();
	/// distance between actual and targeted position during last up/down in text
	int textTargetOffset() const;

	/// access to owning BufferView
	BufferView & bv() const;
	/// reset cursor bottom to the beginning of the top inset
	// (sort of 'chroot' environment...)
	void reset();

	/**
	 * the event was not (yet) dispatched.
	 *
	 * Should only be called by an inset's doDispatch() method. It means:
	 * I, the doDispatch() method of InsetFoo, hereby declare that I am
	 * not able to handle that request and trust my parent will do the
	 * Right Thing (even if my getStatus partner said that I can do it).
	 * It is sort of a kludge that should be used only rarely...
	 */
	void undispatched() const;
	/// the event was already dispatched
	void dispatched() const;

	/// Describe the screen update that should be done
	void screenUpdateFlags(Update::flags f) const;
	/**
	 * Reset update flags to Update::None.
	 *
	 * Should only be called by an inset's doDispatch() method. It means:
	 * I handled that request and I can reassure you that the screen does
	 * not need to be re-drawn and all entries in the coord cache stay
	 * valid (and there are no other things to put in the coord cache).
	 * This is a fairly rare event as well and only some optimization.
	 * Not using noScreenUpdate() should never be wrong.
	 */
	void noScreenUpdate() const;

	/// Forces an updateBuffer() call
	void forceBufferUpdate() const;
	/// Removes any pending updateBuffer() call
	void clearBufferUpdate() const;
	/// Do we need to call updateBuffer()?
	bool needBufferUpdate() const;

	/// Repopulate the slices insets from bottom to top. Useful
	/// for stable iterators or Undo data.
	void sanitize();

	///
	void checkBufferStructure();

	/// Determine if x falls to the left or to the side of the middle of the
	/// inset, and advance the cursor to match this position. If edit is true,
	/// keep the cursor in front of the inset if it matter for dialogs.
	/// Note: it does not handle RTL text yet, and is only used in math for now.
	void moveToClosestEdge(int x, bool edit = false);

	/// whether the cursor is either at the first or last row
	bool atFirstOrLastRow(bool up);

public:
//private:

	///
	DocIterator const & beforeDispatchCursor() const { return beforeDispatchCursor_; }
	///
	void saveBeforeDispatchPosXY();

private:
	///
	BufferView * bv_;
	///
	mutable DispatchResult disp_;
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
	/// if a x_target cannot be hit exactly in a text, put the difference here
	int textTargetOffset_;
	/// position before dispatch started
	DocIterator beforeDispatchCursor_;
	/// cursor screen coordinates before dispatch started
	int beforeDispatchPosX_;
	int beforeDispatchPosY_;

///////////////////////////////////////////////////////////////////
//
// The part below is the non-integrated rest of the original math
// cursor. This should be either generalized for texted or moved
// back to the math insets.
//
///////////////////////////////////////////////////////////////////

public:
	/// return false for empty math insets
	/// Use force to skip the confirmDeletion check.
	bool erase(bool force = false);
	bool backspace(bool force = false);

	/// move the cursor up by sending an internal LFUN_UP
	/// return true if fullscreen update is needed
	bool up();
	/// move the cursor up by sending an internal LFUN_DOWN,
	/// return true if fullscreen update is needed
	bool down();
	/// move up/down in a text inset, called for LFUN_UP/DOWN,
	/// return true if successful, updateNeeded set to true if fullscreen
	/// update is needed, otherwise it's not touched
	bool upDownInText(bool up, bool & updateNeeded);
	/// move up/down in math or any non text inset, call for LFUN_UP/DOWN
	/// return true if successful
	bool upDownInMath(bool up);
	/// move forward in math. word: whether to skip a whole "word" (insets with
	/// the same mathclass)
	bool mathForward(bool word);
	///
	bool mathBackward(bool word);
	///
	void plainErase();
	///
	void plainInsert(MathAtom const & at);

	/// interpret name of a macro or ditch it if \c cancel is true.
	/// Returns true if something got inserted.
	bool macroModeClose(bool cancel = false);
	/// are we currently typing the name of a macro?
	bool inMacroMode() const;
	/// get access to the macro we are currently typing
	InsetMathUnknown * activeMacro();
	/// get access to the macro we are currently typing
	InsetMathUnknown const * activeMacro() const;
	/// the name of the macro we are currently inputting
	docstring macroName();

	/// replace selected stuff with at, placing the former
	// selection in entry cell of atom
	void handleNest(MathAtom const & at);

	/// make sure cursor position is valid
	/// FIXME: It does a subset of fixIfBroken. Maybe merge them?
	void normalize();

	/// injects content of a cell into parent
	void pullArg();
	/// split font inset etc
	void handleFont(std::string const & font);

	/// can we enter the inset?
	bool openable(MathAtom const &) const;
	/// font at cursor position
	Font getFont() const;
};


/**
 * Notifies all insets which appear in \c old, but not in \c cur. And then
 * notify all insets which appear in \c cur, but not in \c old.
 * \returns true if cursor is now invalid, e.g. if some insets in
 *   higher cursor slices of \c old do not exist anymore. In this case
 *   it may be necessary to use Use Cursor::fixIfBroken.
 */
bool notifyCursorLeavesOrEnters(Cursor const & old, Cursor & cur);


} // namespace lyx

#endif // LCURSOR_H
