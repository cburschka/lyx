// -*- C++ -*-
/**
 * \file LyXView.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjornes <larsbj@lyx.org>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef LYXVIEW_H
#define LYXVIEW_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include "support/types.h"

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>

class Buffer;
class Toolbar;
class MiniBuffer;
class Intl;
class Menubar;

class BufferView;
class Dialogs;
class LyXFunc;
class LyXFont;
class Timeout;

/**
 * LyXView - main LyX window
 *
 * This class represents the main LyX window and provides
 * accessor functions to its content.
 *
 * The eventual intention is that LyX will support a number
 * of containing LyXViews. Currently a lot of code still
 * relies on there being a single top-level view.
 *
 * Additionally we would like to support multiple views
 * in a single LyXView.
 */
class LyXView : public boost::signals::trackable, boost::noncopyable {
public:
	///
	LyXView();
	///
	virtual ~LyXView();
	///
	virtual void init() = 0;
	///
	virtual void setPosition(int, int) = 0;
	///
	virtual void show(int, int, string const &) = 0;
	/// Redraw the main form.
	virtual void redraw() = 0;

	/// Resize all BufferViews in this LyXView (because the width changed)
	void resize();

	/// returns the buffer currently shown in the main form.
	Buffer * buffer() const;

	///
	BufferView * view() const;

	/// return the toolbar for this view
	Toolbar * getToolbar() const;

	/// sets the layout in the toolbar layout combox
	void setLayout(string const & layout);
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

	/// return the menubar for this view
	Menubar * getMenubar() const;

	///
	void updateMenubar();

	///
	Intl * getIntl() const;

	/// get access to the dialogs
	Dialogs * getDialogs() { return dialogs_.get(); }

	///
	void updateLayoutChoice();

	/// Updates the title of the window
	void updateWindowTitle();

	/// Show state (toolbar and font in minibuffer)
	void showState();

	/// Reset autosave timer
	void resetAutosaveTimer();
	///
	virtual void prohibitInput() const = 0;
	///
	virtual void allowInput() const = 0;
 
protected:
	/// view of a buffer. Eventually there will be several.
	boost::scoped_ptr<BufferView> bufferview_;

	/// view's menubar
	boost::scoped_ptr<Menubar> menubar_;
	/// view's toolbar
	boost::scoped_ptr<Toolbar> toolbar_;
	/// view's minibuffer
	boost::scoped_ptr<MiniBuffer> minibuffer_;

	/// keyboard mapping object
	boost::scoped_ptr<Intl> intl_;

	/// auto-saving of buffers
	boost::scoped_ptr<Timeout> autosave_timeout_;

	/// called on timeout
	void AutoSave();

	/// FIXME: GUII - toolbar property
	void invalidateLayoutChoice();

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(string const & t, string const & it) = 0;

	/// our function handler
	boost::scoped_ptr<LyXFunc> lyxfunc_;
	/// dialogs for this view
	boost::scoped_ptr<Dialogs> dialogs_;

	/**
	 * The last textclass layout list in the layout choice selector
	 * This should probably be moved to the toolbar, but for now it's
	 * here. (Asger)
	 *
	 * FIXME: GUII
	 */
	int last_textclass_;
};

#endif // LYXVIEW_H
