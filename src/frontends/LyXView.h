// -*- C++ -*-
/**
 * \file LyXView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXVIEW_H
#define LYXVIEW_H

#include "frontends/Application.h"
#include "frontends/Toolbars.h"

#include "lyxfunc.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/utility.hpp>

namespace lyx {

class Buffer;
class InsetBase;
class Menubar;

class BufferView;
class Dialogs;
class LyXFunc;
class LyXFont;
class Timeout;
class FuncRequest;

namespace frontend {
class WorkArea;
class ControlCommandBuffer;
} // namespace frontend


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

	void setWorkArea(frontend::WorkArea * work_area);

	/**
	 * This is called after the concrete view has been created.
	 * We have to have the toolbar and the other stuff created
	 * before we can populate it with this call.
	 */
	virtual void init() = 0;

	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		bool maximize) = 0;

	/// show busy cursor
	virtual void busy(bool) const = 0;

	virtual Toolbars::ToolbarPtr makeToolbar(ToolbarBackend::Toolbar const & tbb) = 0;

	//@{ generic accessor functions

	/** return the current buffer view
	    Returned as a shared_ptr so that anything wanting to cache the
	    buffer view can do so safely using a boost::weak_ptr.
	 */
	BufferView * view() const;

	/// return the buffer currently shown in this window
	Buffer * buffer() const;

	/// return the toolbar for this view
	Toolbars & getToolbars() { return *toolbars_.get(); }
	///
	Toolbars const & getToolbars() const { return *toolbars_.get(); }

	/// return the menubar for this view
	Menubar & getMenubar() { return *menubar_.get(); }
	///
	Menubar const & getMenubar() const { return *menubar_.get(); }

	/// get access to the dialogs
	Dialogs & getDialogs() { return *dialogs_.get(); }
	///
	Dialogs const & getDialogs() const { return *dialogs_.get(); }

	//@}

	/// load a buffer into the current workarea
	bool loadLyXFile(std::string const &  name, bool tolastfiles = true);

	/// set a buffer to the current workarea
	void setBuffer(Buffer * b);

	/// updates the possible layouts selectable
	void updateLayoutChoice();

	/// update the toolbar
	void updateToolbars();
	/// update the menubar
	void updateMenubar();
	/// update the status bar
	virtual void updateStatusBar() = 0;

	/// focus the command buffer (minibuffer)
	boost::signal<void()> focus_command_buffer;

	/// display a message in the view
	virtual void message(docstring const &) = 0;

	/// clear any temporary message and replace with current status
	virtual void clearMessage() = 0;

	/// updates the title of the window
	void updateWindowTitle();

	/// reset autosave timer
	void resetAutosaveTimer();

	/// dispatch to current BufferView
	void dispatch(FuncRequest const & cmd);

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	Buffer const * const updateInset(InsetBase const *) const;

	/// returns true if this view has the focus.
	virtual bool hasFocus() const = 0;

	/// Temporary method to access the current workArea.
	/// This is needed for the gtk frontend.
	frontend::WorkArea * workArea();

	/// show the error list to the user
	void showErrorList(std::string const &);

	/// connect to signals in the given BufferView
	void connectBufferView(BufferView & bv);
	/// disconnect from signals in the given BufferView
	void disconnectBufferView();

protected:
	/// current work area (screen view of a BufferView).
	/**
	\todo FIXME: there is only one workArea per LyXView for now.
	*/
	frontend::WorkArea * work_area_;

	/// view's menubar
	boost::scoped_ptr<Menubar> menubar_;

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(docstring const & t, docstring const & it) = 0;

	/// called on timeout
	void autoSave();

	/// view's toolbar
	boost::scoped_ptr<Toolbars> toolbars_;
	/// auto-saving of buffers
	boost::scoped_ptr<Timeout> const autosave_timeout_;
	/// our function handler
	boost::scoped_ptr<LyXFunc> lyxfunc_;
	/// dialogs for this view
	boost::scoped_ptr<Dialogs> dialogs_;

	/// buffer changed signal connection
	boost::signals::connection bufferChangedConnection_;
	/// buffer errors signal connection
	boost::signals::connection errorsConnection_;
	/// buffer messages signal connection
	boost::signals::connection messageConnection_;
	/// buffer busy status signal connection
	boost::signals::connection busyConnection_;
	/// buffer title changed signal connection
	boost::signals::connection titleConnection_;
	/// buffer reset timers signal connection
	boost::signals::connection timerConnection_;
	/// buffer readonly status changed signal connection
	boost::signals::connection readonlyConnection_;
	/// buffer closing signal connection
	boost::signals::connection closingConnection_;
	/// connect to signals in the given buffer
	void connectBuffer(Buffer & buf);
	/// disconnect from signals in the given buffer
	void disconnectBuffer();

	/// BufferView messages signal connection
	//@{
	boost::signals::connection message_connection_;
	boost::signals::connection show_dialog_connection_;
	boost::signals::connection show_dialog_with_data_connection_;
	boost::signals::connection show_inset_dialog_connection_;
	boost::signals::connection update_dialog_connection_;
	boost::signals::connection layout_changed_connection_;
	//@}

	/// Bind methods for BufferView messages signal connection
	//@{
	void showDialog(std::string const & name);
	void showDialogWithData(std::string const & name,
		std::string const & data);
	void showInsetDialog(std::string const & name,
		std::string const & data, InsetBase * inset);
	void updateDialog(std::string const & name,
		std::string const & data);
	//@}

	/// notify readonly status
	void showReadonly(bool);

protected:
	/// view's command buffer controller
	// this has to be declared _after_ lyxfunc_ as its initialization depends
	// on it!
	typedef boost::scoped_ptr<frontend::ControlCommandBuffer>
	CommandBufferPtr;

	CommandBufferPtr const controlcommand_;
};

} // namespace lyx

#endif // LYXVIEW_H
