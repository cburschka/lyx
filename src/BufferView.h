// -*- C++ -*-
/**
 * \file BufferView.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 *  \author Lars Gullik Bjønnes
*/

#ifndef BUFFER_VIEW_H
#define BUFFER_VIEW_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "undo.h"

#include "insets/inset.h"

#include <boost/utility.hpp>

class LyXView;
class LyXText;
class TeXErrors;
class Buffer;
class LyXScreen;
class Language;
class Painter;
class UpdatableInset;
class WordLangTuple;

/**
 * A buffer view encapsulates a view onto a particular
 * buffer, and allows access to operate upon it. A view
 * is a sliding window of the entire document rendering.
 *
 * Eventually we will allow several views onto a single
 * buffer, but not yet.
 */
class BufferView : boost::noncopyable {
public:
	/**
	 * Codes to store necessary pending updates
	 * of the document rendering.
	 */
	enum UpdateCodes {
		UPDATE = 0, //< FIXME
		SELECT = 1, //< selection change
		FITCUR = 2, //< the cursor needs fitting into the view
		CHANGE = 4  //< document data has changed
	};

	/**
	 * Create a view with the given owner main window,
	 * of the given dimensions.
	 */
	BufferView(LyXView * owner, int x, int y, int w, int h);

	~BufferView();
 
	/// set the buffer we are viewing
	void buffer(Buffer * b);
	/// return the buffer being viewed
	Buffer * buffer() const;
 
	/// return the painter object for drawing onto the view
	Painter & painter() const;
	/// return the screen object for handling re-drawing
	LyXScreen & screen() const;
	/// return the owning main view
	LyXView * owner() const;
 
	/// resize event has happened
	void resize();
	/**
	 * Repaint the pixmap. Used for when we don't want
	 * to go through the full update() logic, just a simple
	 * repaint of the whole screen.
	 */
	void repaint();
 
	/// fit the user cursor within the visible view
	bool fitCursor();
	/// perform pending painting updates
	void update();
	// update for a particular lyxtext
	void update(LyXText *, UpdateCodes uc);
	/// update for a particular inset
	void updateInset(Inset * inset, bool mark_dirty);
	/// reset the scrollbar to reflect current view position
	void updateScrollbar();
	/// FIXME
	void redoCurrentBuffer();
 
	/// FIXME
	bool available() const;
 
	/// FIXME
	void beforeChange(LyXText *);
 
	/// Save the current position as bookmark i
	void savePosition(unsigned int i);
	/// Restore the position from bookmark i
	void restorePosition(unsigned int i);
	/// does the given bookmark have a saved position ?
	bool isSavedPosition(unsigned int i);
 
	/**
	 * This holds the mapping between buffer paragraphs and screen rows.
	 * This should be private...but not yet. (Lgb)
	 */
	LyXText * text;
	/// return the lyxtext we are using
	LyXText * getLyXText() const;
 
	/// Return the current inset we are "locked" in
	UpdatableInset * theLockingInset() const;
	/// lock the given inset FIXME: return value ?
	bool lockInset(UpdatableInset * inset);
	/// unlock the given inset
	int unlockInset(UpdatableInset * inset);
	/// unlock the currently locked inset
	void insetUnlock();
 
	/// return the parent language of the given inset
	Language const * getParentLanguage(Inset * inset) const;
 
	/// Select the "current" word
	void selectLastWord();
	/// replace the currently selected word
	void replaceWord(string const & replacestring);
	/// Update after spellcheck finishes
	void endOfSpellCheck();
	/// return the next word
	WordLangTuple const nextWord(float & value);
 
	/// move cursor to the named label
	bool gotoLabel(string const & label);
 
	/// copy the environment type from current paragraph
	void copyEnvironment();
	/// set the current paragraph's environment type
	void pasteEnvironment();
 
	/// undo last action
	void undo();
	/// redo last action
	void redo();
 
	/// removes all autodeletable insets
	bool removeAutoInsets();
	/// insert all errors found when running latex
	void insertErrors(TeXErrors & terr);
	/// set the cursor based on the given TeX source row
	void setCursorFromRow(int row);
 
	/**
	 * Insert an inset into the buffer.
	 * Place it in a layout of lout,
	 */
	bool insertInset(Inset * inset, string const & lout = string());
 
	/// Inserts a lyx file at cursor position. return false if it fails
	bool insertLyXFile(string const & file);
 
	/// show the user cursor
	void showCursor();
	/// hide the user cursor
	void hideCursor();
	/// FIXME
	void showLockedInsetCursor(int x, int y, int asc, int desc);
	/// FIXME
	void hideLockedInsetCursor();
	/// FIXME
	bool fitLockedInsetCursor(int x, int y, int asc, int desc);
	/// FIXME
	void lockedInsetStoreUndo(Undo::undo_kind kind);
	/// FIXME
	void toggleSelection(bool = true);
	/// FIXME: my word !
	void toggleToggle();
 
	/// center the document view around the cursor
	void center();
	/// scroll document by the given number of lines of default height
	void scroll(int lines);
	/// Scroll the view by a number of pixels
	void scrollDocView(int);

	/// height of a normal line in pixels (zoom factor considered)
	int defaultHeight() const;
	/// return the pixel width of the document view
	int workWidth() const;
	/// return the pixel height of the document view
	int workHeight() const;

	/// switch between primary and secondary keymaps for RTL entry
	void switchKeyMap();

	/// FIXME
	bool ChangeRefsIfUnique(string const & from, string const & to);
	/// FIXME
	bool ChangeCitationsIfUnique(string const & from, string const & to);
 
	/// get the contents of the window system clipboard
	string const getClipboard() const;
	/// fill the window system clipboard
	void stuffClipboard(string const &) const;
	/// tell the window system we have a selection
	void haveSelection(bool sel);
 
	/// execute the given function
	bool dispatch(FuncRequest const & argument);
 
private:
	/// Set the current locking inset
	void theLockingInset(UpdatableInset * inset);
 
	/// return the lyxtext containing this inset
	LyXText * getParentText(Inset * inset) const;
 
	/**
	 * Change all insets with the given code's contents to a new
	 * string. May only be used with InsetCommand-derived insets
	 * Returns true if a screen update is needed.
	 */
	bool ChangeInsets(Inset::Code code, string const & from,
			  string const & to);
 

	struct Pimpl;
	friend struct BufferView::Pimpl;

	Pimpl * pimpl_;
};


BufferView::UpdateCodes operator|(BufferView::UpdateCodes uc1,
				  BufferView::UpdateCodes uc2);

#endif // BUFFERVIEW_H
