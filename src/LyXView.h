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

#ifndef LyXView_H
#define LyXView_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION

#include "Timeout.h"

class LyXFunc;
class Toolbar;
class MiniBuffer;
class Intl;
class Buffer;
class Menus;
class BufferView;

///
struct  FD_form_main {
	///
	FL_FORM * form_main;
#if 0
	///
	FL_OBJECT * timer_autosave;
#endif
#if 0
	///
	FL_OBJECT * timer_update;
#endif
#if 0
	///
	void * vdata;
	///
	long ldata;
#endif
};


/**
  This class is the form containing the view of the buffer. The actual buffer
  view is supposed (at least IMHO) to be another class, that shows its output
  in one or more LyXView's.
 */
class LyXView {
public:
	/// constructor
	LyXView(int w, int h);

	/// destructor
	~LyXView();

	/// Where to place the form.
	void setPosition(int, int);

	/// Show the main form.
	void show(int, int, char const * t = "LyX");

	/// init (should probably be removed later) (Lgb)
	void init();

	/// Redraw the main form.
	void redraw();

	/// returns the buffer currently shown in the main form.
	Buffer * buffer() const;

	///
	BufferView * view() const;

	/// returns a pointer to the main form.
	FD_form_main * getMainForm() const;

	/// returns a pointer to the form.
	FL_FORM * getForm() const;

	/// return a pointer to the toolbar
	Toolbar * getToolbar() const;

	/// return a pointer to the lyxfunc
	LyXFunc * getLyXFunc() const;

	/// return a pointer to the minibuffer
	MiniBuffer * getMiniBuffer() const;

	///
	Menus * getMenus() const;

	///
	Intl * getIntl() const;

	///
	void updateLayoutChoice();

	/// Updates the title of the window
	void updateWindowTitle();

	/// Reset autosave timer
	void resetAutosaveTimer();
private:
	/// 
	LyXFunc * lyxfunc;
	/// 
	Toolbar * toolbar;
	/// 
	MiniBuffer * minibuffer;
	///
	Menus * menus;
	///
	Intl * intl;

	/** This is supposed to be a pointer or a list of pointers to the
	   BufferViews currently being shown in the LyXView. So far
	   this is not used, but that should change pretty soon. (Lgb) */
	BufferView * bufferview;
	///
	void invalidateLayoutChoice();
	///
	void UpdateDocumentClassChoice();
public:
 	///
 	static int KeyPressMask_raw_callback(FL_FORM *, void * xev);
	/** This callback is run when a close event is sent from the
	  window manager. */
	static int atCloseMainFormCB(FL_FORM *, void *);
	/// A callback
	void AutoSave();
	/// A callback
	static void UpdateTimerCB(void *);
#if 1
	///
	//Timeout update_timeout;
#endif
private:
	///
	Timeout autosave_timeout;
	/// makes the main form.
	void create_form_form_main(int width, int height);
	/// A pointer to the form.	
	FD_form_main * form_main_;
	/// A pointer to the form.	
	FL_FORM * form_;
	/** The last textclass layout list in the layout choice selector
	  This should probably be moved to the toolbar, but for now it's
	here. (Asger) */
	int last_textclass;
};
#endif
