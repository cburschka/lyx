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

#include "frontends/LyXView.h"

/**
 * XFormsView - xforms implementation of LyXView
 *
 * xforms-private implementation of the main LyX window.
 */
class XFormsView : public LyXView {
public:
	/// create a main window of the given dimensions
	XFormsView(int w, int h);

	~XFormsView();
	/// Where to place the form.
	virtual void setPosition(int, int);
	/// Show the main form.
	virtual void show(int, int, string const & t = string("LyX"));
	/// init (should probably be removed later) (Lgb)
	virtual void init();
	/// get the xforms main form
	FL_FORM * getForm() const;
	/// redraw the main form.
	virtual void redraw();
	///
	virtual void prohibitInput() const;
	///
	virtual void allowInput() const;

	/// callback for close event from window manager
	static int atCloseMainFormCB(FL_FORM *, void *);

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(string const & t, string const & it); 
 
	/// makes the main form.
	void create_form_form_main(int width, int height);
	/// the main form.
	boost::scoped_ptr<FL_FORM> form_;
};
#endif
