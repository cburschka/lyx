// -*- C++ -*-
/**
 * \file LyXView.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes <larsbj@lyx.org>
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
#include <boost/shared_ptr.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signals/signal0.hpp>

class Buffer;
class Toolbar;
class Intl;
class Menubar;
class ControlCommandBuffer;

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

	LyXView();

	virtual ~LyXView();

	/**
	 * This is called after the concrete view has been created.
	 * We have to have the toolbar and the other stuff created
	 * before we can populate it with this call.
	 */
	void init();

	/// start modal operation
	virtual void prohibitInput() const = 0;
	/// end modal operation
	virtual void allowInput() const = 0;

	//@{ generic accessor functions

	/** return the current buffer view
	    Returned as a shared_ptr so that anything wanting to cache the
	    buffer view can do so safely using a boost::weak_ptr.
	 */
	boost::shared_ptr<BufferView> const & view() const;

	/// return the LyX function handler for this view
	LyXFunc * getLyXFunc() const;

	/// return the buffer currently shown in this window
	Buffer * buffer() const;

	/// return the toolbar for this view
	Toolbar * getToolbar() const;

	/// return the menubar for this view
	Menubar * getMenubar() const;

	/// get access to the dialogs
	Dialogs * getDialogs() { return dialogs_.get(); }

	/// get this view's keyboard map handler
	Intl * getIntl() const;

	//@}

	/// sets the layout in the toolbar layout selection
	void setLayout(string const & layout);
	/// updates the possible layouts selectable
	void updateLayoutChoice();

	/// update the toolbar
	void updateToolbar();
	/// update the menubar
	void updateMenubar();

	/// focus the command buffer (minibuffer)
	boost::signal0<void> focus_command_buffer;
 
	/// view state string changed
	boost::signal0<void> view_state_changed;

	/// display a message in the view
	virtual void message(string const &) = 0;

	/// updates the title of the window
	void updateWindowTitle();

	/// reset autosave timer
	void resetAutosaveTimer();

protected:
	/// view of a buffer. Eventually there will be several.
	boost::shared_ptr<BufferView> bufferview_;

	/// view's menubar
	boost::scoped_ptr<Menubar> menubar_;
	/// view's toolbar
	boost::scoped_ptr<Toolbar> toolbar_;
	/// view's command buffer controller
	boost::scoped_ptr<ControlCommandBuffer> controlcommand_;

	/// keyboard mapping object
	boost::scoped_ptr<Intl> intl_;

	/// auto-saving of buffers
	boost::scoped_ptr<Timeout> autosave_timeout_;

	/// called on timeout
	void autoSave();

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
};

#endif // LYXVIEW_H
