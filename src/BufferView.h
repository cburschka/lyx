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
	FL_OBJECT * getWorkArea() { return work_area; }
	///
	void buffer(Buffer * b);
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
	LyXScreen * getScreen() {
		fl_set_timer(timer_cursor, 0.4);
		return screen;
	}
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
	void hideLockedInsetCursor(long x, long y, int asc, int desc);
	///
	void fitLockedInsetCursor(long x, long y, int asc, int desc);
	///
	int unlockInset(UpdatableInset * inset);
	///
	void lockedInsetStoreUndo(Undo::undo_kind kind);
private:
	/// Update pixmap of screen
	void updateScreen();
	///
	int workAreaExpose();
	///
	void create_view(int, int, int, int);
	///
	Inset * checkInsetHit(int & x, int & y);
	/// 
	int ScrollUp(long time);
	///
	int ScrollDown(long time);
	///
	void ScrollUpOnePage(long /*time*/);
	///
	void ScrollDownOnePage(long /*time*/);

public:
	/// A callback for the up arrow in the scrollbar.
	static void UpCB(FL_OBJECT * ob, long);

	/// A callback for the slider in the scrollbar.
	static void ScrollCB(FL_OBJECT * ob, long);

	/// A callback for the down arrow in the scrollbar.
	static void DownCB(FL_OBJECT * ob, long);

	///
	static void CursorToggleCB(FL_OBJECT * ob, long);
	/** Work area free object handler
	 */
	static int work_area_handler(FL_OBJECT *, int event,
				     FL_Coord, FL_Coord, int key, void *xev);

	///
	void SetState();

private:
	///
	int WorkAreaMotionNotify(FL_OBJECT * ob,
				 Window win,
				 int w, int h,
				 XEvent * ev, void * d);
	///
	int WorkAreaSelectionNotify(FL_OBJECT *, Window win,
				    int /*w*/, int /*h*/,
				    XEvent * event, void * /*d*/);
	///
	int WorkAreaButtonPress(FL_OBJECT * ob,
				Window win,
				int w, int h,
				XEvent * ev, void * d);
	///
	int WorkAreaButtonRelease(FL_OBJECT * ob,
				  Window win,
				  int w, int h,
				  XEvent * ev, void * d);
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
	FL_OBJECT * work_area;
	///
	FL_OBJECT * figinset_canvas;
	///
	FL_OBJECT * scrollbar;
	///
	FL_OBJECT * button_down;
	///
	FL_OBJECT * button_up;
	///
	FL_OBJECT * timer_cursor;
        ///
        BackStack backstack;
	///
	int last_click_x, last_click_y;
};

#endif
