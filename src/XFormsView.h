// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LyXView_H
#define LyXView_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION

#include "LyXView.h"

/**
  This class is the form containing the view of the buffer. The actual buffer
  view is supposed (at least IMHO) to be another class, that shows its output
  in one or more LyXView's.
 */
class XFormsView : public LyXView {
public:
	/// constructor
	XFormsView(int w, int h);
	/// destructor
	~XFormsView();
	/// Where to place the form.
	virtual void setPosition(int, int);
	/// Show the main form.
	virtual void show(int, int, string const & t = string("LyX"));
	/// init (should probably be removed later) (Lgb)
	virtual void init();
	/// Redraw the main form.
	virtual void redraw();
	/// returns a pointer to the form.
	FL_FORM * getForm() const;
	///
	virtual void prohibitInput() const;
	///
	virtual void allowInput() const;
	/** This callback is run when a close event is sent from the
	  window manager. */
	static int atCloseMainFormCB(FL_FORM *, void *);
private:
	/// pass the title, and the iconified title
	virtual void setWindowTitle(string const &, string const &);
	/// makes the main form.
	void create_form_form_main(int width, int height);
	/// A pointer to the form.	
	FL_FORM * form_;
};
#endif
