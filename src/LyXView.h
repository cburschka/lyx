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

#include <config.h>
#include FORMS_H_LOCATION

#include "LString.h"
#include "frontends/Timeout.h"
#include <boost/utility.hpp>
#include "layout.h"

class LyXFunc;
class Toolbar;
class MiniBuffer;
class Intl;
class Buffer;
class Menubar;

class BufferView;
class Dialogs;

/**
  This class is the form containing the view of the buffer. The actual buffer
  view is supposed (at least IMHO) to be another class, that shows its output
  in one or more LyXView's.
 */
class LyXView : public SigC::Object, boost::noncopyable {
public:
	/// constructor
	LyXView(int w, int h);

	/// destructor
	~LyXView();

	/// Where to place the form.
	void setPosition(int, int);

	/// Show the main form.
	void show(int, int, string const & t = string("LyX"));

	/// init (should probably be removed later) (Lgb)
	void init();

	/// Redraw the main form.
	void redraw();

	/// returns the buffer currently shown in the main form.
	Buffer * buffer() const;

	///
	BufferView * view() const;

	/// returns a pointer to the form.
	FL_FORM * getForm() const;

	/// return a pointer to the toolbar
	Toolbar * getToolbar() const;

	/// sets the layout in the toolbar layout combox
	void setLayout(LyXTextClass::size_type layout);

	/// update the toolbar
	void updateToolbar();

	/// return a pointer to the lyxfunc
	LyXFunc * getLyXFunc() const;

	/// return a pointer to the minibuffer
	MiniBuffer * getMiniBuffer() const;

	///
	void message(string const &);
	///
	void messagePush(string const & str);
	///
	void messagePop();
	
	///
	Menubar * getMenubar() const;

	///
	void updateMenubar();

	///
	Intl * getIntl() const;

	///
	Dialogs * getDialogs() { return dialogs_; }

	///
	void updateLayoutChoice();

	/// Updates the title of the window
	void updateWindowTitle();

	/// Show state (toolbar and font in minibuffer)
	void showState();

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
	Menubar * menubar;
	///
	Intl * intl;
	///
	Dialogs * dialogs_;

	/** This is supposed to be a pointer or a list of pointers to the
	   BufferViews currently being shown in the LyXView. So far
	   this is not used, but that should change pretty soon. (Lgb) */
	BufferView * bufferview;
	///
	void invalidateLayoutChoice();
public:
	/** This callback is run when a close event is sent from the
	  window manager. */
	static int atCloseMainFormCB(FL_FORM *, void *);
	/// A callback
	void AutoSave();
private:
	///
	Timeout autosave_timeout;
	/// makes the main form.
	void create_form_form_main(int width, int height);
	/// A pointer to the form.	
	FL_FORM * form_;
	/** The last textclass layout list in the layout choice selector
	  This should probably be moved to the toolbar, but for now it's
	here. (Asger) */
	int last_textclass;
};
#endif
