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
class WorkArea;

///
class BufferView : boost::noncopyable {
public:
	///
	enum UpdateCodes {
		///
		UPDATE = 0,
		///
		SELECT = 1,
		///
		FITCUR = 2,
		///
		CHANGE = 4
	};

	///
	BufferView(LyXView * owner, int , int , int, int);
	///
	~BufferView();
	///
	Buffer * buffer() const;
	///
	Painter & painter() const;
	///
	LyXScreen & screen() const;
	/// return the work area for this bview
	WorkArea & workarea() const;
	///
	void buffer(Buffer * b);
	///
	void resize();
	/**
	 * Repaint the pixmap. Used for when we don't want
	 * to go through the full update() logic, just a simple
	 * repaint of the whole screen.
	 */
	void repaint();
	///
	bool fitCursor();
	///
	void update();
	//
	void update(LyXText *, UpdateCodes uc);
	///
	void updateScrollbar();
	///
	Inset * checkInsetHit(LyXText *, int & x, int & y);
	///
	void redoCurrentBuffer();
	///
	void cursorPrevious(LyXText *);
	///
	void cursorNext(LyXText *);
	///
	bool available() const;
	///
	LyXView * owner() const;
	///
	void beforeChange(LyXText *);
	///
	void finishChange(bool fitcur = false);
	///
	void savePosition(unsigned int i);
	///
	void restorePosition(unsigned int i);
	///
	bool isSavedPosition(unsigned int i);
	/** This holds the mapping between buffer paragraphs and screen rows.
	    This should be private...but not yet. (Lgb)
	*/
	LyXText * text;
	///
	LyXText * getLyXText() const;
	///
	LyXText * getParentText(Inset * inset) const;
	///
	Language const * getParentLanguage(Inset * inset) const;
	///
	int workWidth() const;
	///
	UpdatableInset * theLockingInset() const;
	///
	void theLockingInset(UpdatableInset * inset);
	///
	void updateInset(Inset * inset, bool mark_dirty);
	///
	int slx;
	///
	int sly;
	///
	void insetUnlock();
	///
	void replaceWord(string const & replacestring);
	///
	void endOfSpellCheck();
	///
	void selectLastWord();
	/// return the next word
	WordLangTuple const nextWord(float & value);
	///
	bool gotoLabel(string const & label);
	///
	void message(string const & msg) const;
	///
	void paste();
	///
	void cut(bool realcut = true);
	///
	void copy();
	///
	void pasteEnvironment();
	///
	void copyEnvironment();
	///
	void menuUndo();
	///
	void menuRedo();
	/// removes all autodeletable insets
	bool removeAutoInsets();
	///
	void insertErrors(TeXErrors & terr);
	///
	void setCursorFromRow(int row);
	/** Insert an inset into the buffer.
	    Placie it in a layout of lout,
	    if no_table make sure that it doesn't end up in a table.
	*/
	//bool insertInset(Inset * inset, string const & lout = string(),
	//		 bool no_table = false);
	bool insertInset(Inset * inset, string const & lout = string());
	/** Inserts a lyx file at cursor position.
	    \return #false# if it fails.
	*/
	bool insertLyXFile(string const & file);
	///
	bool lockInset(UpdatableInset * inset);
	///
	void showLockedInsetCursor(int x, int y, int asc, int desc);
	///
	void hideLockedInsetCursor();
	///
	bool fitLockedInsetCursor(int x, int y, int asc, int desc);
	///
	int unlockInset(UpdatableInset * inset);
	///
	void lockedInsetStoreUndo(Undo::undo_kind kind);
	///
	void showCursor();
	///
	void hideCursor();
	///
	void toggleSelection(bool = true);
	///
	void toggleToggle();
	///
	void center();

	/// Scroll the view by a number of pixels
	void scrollDocView(int);

	///
	void switchKeyMap();

	///
	bool ChangeInsets(Inset::Code code, string const & from,
			  string const & to);
	///
	bool ChangeRefsIfUnique(string const & from, string const & to);
	///
	bool ChangeCitationsIfUnique(string const & from, string const & to);
	///
	string const getClipboard() const;
	///
	void pasteClipboard(bool asPara);
	///
	void stuffClipboard(string const &) const;
	///
	bool dispatch(FuncRequest const & argument);
	///
	void moveCursorUpdate(bool selecting, bool fitcur = true);
private:
	///
	struct Pimpl;
	///
	friend struct BufferView::Pimpl;
	///
	Pimpl * pimpl_;
};


///
BufferView::UpdateCodes operator|(BufferView::UpdateCodes uc1,
				  BufferView::UpdateCodes uc2);

#endif
