// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef BUFFER_VIEW_H
#define BUFFER_VIEW_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "undo.h"
#include <boost/utility.hpp>

class LyXView;
class LyXText;
class TeXErrors;
class Buffer;
class LyXScreen;
class WorkArea;
class Language;

///
class BufferView : public noncopyable {
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
	Painter & painter();
	///
	LyXScreen * screen() const;
	///
	WorkArea * workarea() const;
	///
	void buffer(Buffer * b);
	///
	void resize(int, int, int, int);
	///
	void resize();
	///
	void redraw();
	///
	void fitCursor(LyXText *);
	///
	void update();
	//
	void update(LyXText *, UpdateCodes uc);
	///
	void updateScrollbar();
	///
	Inset * checkInsetHit(LyXText *, int & x, int & y,
			      unsigned int button);
	/// 
	void redoCurrentBuffer();
	///
	int resizeCurrentBuffer();
	///
	void gotoInset(std::vector<Inset::Code> const & codes,
		       bool same_content);
	///
	void gotoInset(Inset::Code codes, bool same_content);
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
	bool inset_slept;
	///
	int slx;
	///
	int sly;
	///
	void insetUnlock();
	///
	void insetSleep();
	///
	void insetWakeup();
	///
	void replaceWord(string const & replacestring);
	///
	void endOfSpellCheck();
	///
	void selectLastWord();
	///
	string const nextWord(float & value);
	///
	void insertCorrectQuote();
	///
	bool gotoLabel(string const & label);
	///
	void paste();
	///
	void cut();
	///
	void copy();
	///
	void pasteEnvironment();
	///
	void copyEnvironment();
	///
	void hfill();
	///
	void protectedBlank(LyXText *);
	///
	void newline();
	///
	void menuSeparator();
	///
	void endOfSentenceDot();
	///
	void ldots();
	///
	void hyphenationPoint();
	///
	void menuUndo();
	///
	void menuRedo();
#ifndef NEW_INSETS
	///
	void toggleFloat();
	///
	void openStuff();
#endif
	///
	void insertNote();
#ifndef NEW_INSETS
	///
	void allFloats(char flag, char figmar);
#endif
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
	bool insertInset(Inset * inset, string const & lout = string(),
			 bool no_table = false);
	/// open and lock an updatable inset
	bool open_new_inset(UpdatableInset * new_inset, bool behind = false);
	/** Inserts a lyx file at cursor position.
	    @return #false# if it fails.
	*/
	bool insertLyXFile(string const & file);
	///
	bool lockInset(UpdatableInset * inset);
	///
	void showLockedInsetCursor(int x, int y, int asc, int desc);
	///
	void hideLockedInsetCursor();
	///
	void fitLockedInsetCursor(int x, int y, int asc, int desc);
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
	
	///
	bool focus() const;
	///
	void focus(bool);
	///
	bool active() const;
	///
	bool belowMouse() const;
	/// A callback for the slider in the scrollbar.
	void scrollCB(double);

	///
	void setState();

	///
	void pushIntoUpdateList(Inset * i);
	///
	bool ChangeInsets(Inset::Code code, string const & from, 
			  string const & to);
	///
	bool ChangeRefsIfUnique(string const & from, string const & to);
	///
	bool ChangeCitationsIfUnique(string const & from, string const & to);
	///
	void pasteClipboard(bool asPara);
	///
	void stuffClipboard(string const &) const;
private:
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
