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

class LyXView;
class LyXText;
class TeXErrors;
class Buffer;
class LyXScreen;
class WorkArea;

///
class BufferView {
public:
	///
	enum UpdateCodes {
		UPDATE = 0,
		SELECT = 1,
		FITCUR = 2,
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
	void fitCursor();
	///
	void update();
	///
	void update(UpdateCodes uc);
	///
	void updateScrollbar();
	///
	void redoCurrentBuffer();
	///
	int resizeCurrentBuffer();
	///
	void gotoError();
	///
	void cursorPrevious();
	///
	void cursorNext();
	/// 
	bool available() const;
	///
	LyXView * owner() const;
	///
	void beforeChange();
        ///
        void savePosition();
        ///
        void restorePosition();
	///
	bool NoSavedPositions();
	/** This holds the mapping between buffer paragraphs and screen rows.
	    This should be private...but not yet. (Lgb)
	*/
	LyXText * text;
	///
	int workWidth() const;
	///
	UpdatableInset * the_locking_inset;
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
	char * nextWord(float & value);
	///
	void insertCorrectQuote();
	///
	void gotoNote();
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
	void protectedBlank();
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
	///
	void toggleFloat();
	///
	void openStuff();
	///
	void insertNote();
	///
	void allFloats(char flag, char figmar);
	/// removes all autodeletable insets
	bool removeAutoInsets();
	///
	void insertErrors(TeXErrors & terr);
	///
	void setCursorFromRow(int row);
	/** Insert an inset into the buffer
	    Insert inset into buffer, placing it in a layout of lout,
	    if no_table make sure that it doesn't end up in a table. */
	bool insertInset(Inset * inset, string const & lout = string(),
			 bool no_table = false);
	/// open and lock an updatable inset
	void open_new_inset(UpdatableInset * new_inset);
	/// Inserts a lyx file at cursor position. Returns false if it fails.
	bool insertLyXFile(string const & file);
	///
	bool lockInset(UpdatableInset * inset);
	///
	void showLockedInsetCursor(long x, long y, int asc, int desc);
	///
	void hideLockedInsetCursor();
	///
	void fitLockedInsetCursor(long x, long y, int asc, int desc);
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
	void workAreaExpose();
	///
	void workAreaButtonPress(int x, int y, unsigned int button);
	///
	void workAreaButtonRelease(int x, int y, unsigned int button);
	///
	void workAreaMotionNotify(int x, int y, unsigned int state);
	///
	void doubleClick(int x, int y, unsigned int button);
	///
	void tripleClick(int x, int y, unsigned int button);
	///
	void enterView();
	///
	void leaveView();
	///
	bool ChangeRefs(string const & from, string const & to);
	///
	void pasteClipboard(bool asPara);
	///
	void stuffClipboard(string const &) const;
private:
	struct Pimpl;
	Pimpl * pimpl_;
};


BufferView::UpdateCodes operator|(BufferView::UpdateCodes uc1,
				  BufferView::UpdateCodes uc2);

#endif
