// -*- C++ -*-
/**
 * \file QtView.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjornes <larsbj@lyx.org>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTVIEW_H
#define QTVIEW_H

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>
 
#include "frontends/LyXView.h"
 
#include <qmainwindow.h>
 
/**
 * QtView - Qt implementation of LyXView
 *
 * Qt-private implementation of the main LyX window.
 */
class QtView : public QMainWindow, public LyXView {
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	QtView(unsigned int w, unsigned int h);

	~QtView();
 
	/// set the x,y of the top level window
	virtual void setPosition(unsigned int, unsigned int);
 
	/**
	 * show - display the top-level window
	 * @param place general placement (FIXME: GUII)
	 * @param border border border type (FIXME: GUII)
	 * @param title window title
	 */
	virtual void show(int, int, string const & t = string("LyX"));
 
	/// start modal operation
	virtual void prohibitInput() const;
	/// end modal operation
	virtual void allowInput() const;
 
	/// redraw the main form.
	void redraw();
 
public slots:
	/// menu item has been selected
	void activated(int id);
 
protected:
	/// make sure we quit cleanly
	virtual void closeEvent(QCloseEvent * e);
 
private:
	/** 
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(string const & t, string const & it) {
		setCaption(t.c_str());
		setIconText(it.c_str());
	}
};
 
#endif // QTVIEW_H
