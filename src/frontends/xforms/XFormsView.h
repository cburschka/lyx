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

#include "forms_fwd.h"

#include "frontends/LyXView.h"

class XMiniBuffer;

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

	/**
	 * show - display the top-level window
	 * @param xpos requested x position (or 0)
	 * @param xpos requested y position (or 0)
	 * @param title window title
	 */
	void show(int xpos, int ypos, string const & t = string("LyX"));

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

	/// display a status message
	virtual void message(string const & str);

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(string const & t, string const & it);

	/// update the minibuffer state message
	void show_view_state();

	/// makes the main form.
	void create_form_form_main(int width, int height);
	/// the minibuffer
	boost::scoped_ptr<XMiniBuffer> minibuffer_;
	///
	boost::signals::connection view_state_con;
	///
	boost::signals::connection focus_con;
	///
	boost::signals::connection redraw_con;

	/// the main form.
	FL_FORM * form_;
};
#endif
