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
#include "BackStack.h"
#include "LaTeX.h"
#include "undo.h"

class LyXView;
class Buffer;
class LyXScreen;
class Inset;
class LyXText;
class WorkArea;

///
class BufferView {
public:
	///
	BufferView(LyXView * owner, int , int , int, int);
	///
	~BufferView();
	///
	Buffer * buffer() const { return buffer_; }
	///
	Painter & painter();
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
	void update(signed char f);
	///
	void smallUpdate(signed char f);
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
	LyXView * owner() const { return owner_; }
	///
	void beforeChange();
        ///
        void savePosition();
        ///
        void restorePosition();
	/** This holds the mapping between buffer paragraphs and screen rows.
	    This should be private...but not yet. (Lgb)
	*/
	LyXText * text;
	///
	UpdatableInset * the_locking_inset;
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
	void insertInset(Inset * inset, string const & lout = string(),
			 bool no_table = false);
	/// open and lock an updatable inset
	void open_new_inset(UpdatableInset * new_inset);
	/// Inserts a lyx file at cursor position. Returns false if it fails.
	bool insertLyXFile(string const & file);
	///
	int lockInset(UpdatableInset * inset);
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
	
private:
	friend class WorkArea;
	
	/// Update pixmap of screen
	void updateScreen();
	///
	void workAreaExpose();
	///
	void scrollUpOnePage();
	///
	void scrollDownOnePage();
	///
	void create_view();
	///
	Inset * checkInsetHit(int & x, int & y);
	/// 
	int scrollUp(long time);
	///
	int scrollDown(long time);

public:
	///
	bool focus() const;
	///
	void focus(bool);
	///
	bool active() const;
	/// A callback for the up arrow in the scrollbar.
	void upCB(long time, int button);
	/// A callback for the slider in the scrollbar.
	void scrollCB(double);
	/// A callback for the down arrow in the scrollbar.
	void downCB(long time, int button);

	///
	static void cursorToggleCB(FL_OBJECT * ob, long);

	///
	void setState();

private:
	///
	void workAreaMotionNotify(int x, int y, unsigned int state);
	///
	void workAreaButtonPress(int x, int y, unsigned int button);
	///
	void workAreaButtonRelease(int x, int y, unsigned int button);
	///
	void workAreaSelectionNotify(Window win, XEvent * event);
	///
	LyXView * owner_;
	///
	Buffer * buffer_;
	///
	LyXScreen * screen;
	///
	long current_scrollbar_value;
	///
	bool lyx_focus;
	///
	bool work_area_focus;
	///
	FL_OBJECT * figinset_canvas;
	///
	FL_OBJECT * timer_cursor;
        ///
        BackStack backstack;
	///
	int last_click_x, last_click_y;
	///
	WorkArea * workarea;
};

#endif
