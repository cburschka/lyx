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
#include "support/docstring.h"

#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/utility.hpp>

#include <vector>

namespace lyx {

namespace support { class FileName; }

class Font;
class Buffer;
class BufferView;
class FuncRequest;
class Inset;
class Timeout;
class ToolbarInfo;

namespace frontend {

class Dialogs;
class WorkArea;
class Toolbar;
class Toolbars;

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
	LyXView(int id);
	///
	virtual ~LyXView();
	///
	int id() const { return id_; }
	///
	virtual void close() = 0;
	///
	virtual void setFocus() = 0;

	///
	virtual WorkArea * workArea(Buffer & buffer) = 0;
	///
	virtual WorkArea * addWorkArea(Buffer & buffer) = 0;
	///
	virtual void setCurrentWorkArea(WorkArea * work_area) = 0;
	///
	virtual void removeWorkArea(WorkArea * work_area) = 0;
	/// return the current WorkArea (the one that has the focus).
	virtual WorkArea const * currentWorkArea() const = 0;
	/// FIXME: This non-const access is needed because of
	/// a mis-designed \c ControlSpellchecker.
	virtual WorkArea * currentWorkArea() = 0;

	/**
	 * This is called after the concrete view has been created.
	 * We have to have the toolbar and the other stuff created
	 * before we can populate it with this call.
	 */
	virtual void init() = 0;

	enum Maximized {
		NotMaximized,
		VerticallyMaximized,
		HorizontallyMaximized,
		CompletelyMaximized
	};

	///
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		int maximize,
		unsigned int iconSizeXY,
		const std::string & geometryArg) = 0;

	/// save the geometry state in the session manager.
	virtual void saveGeometry() = 0;

	/// show busy cursor
	virtual void busy(bool) = 0;

	virtual Toolbar * makeToolbar(ToolbarInfo const & tbinfo, bool newline) = 0;

	//@{ generic accessor functions

	/// \return the current buffer view.
	BufferView * view();

	/// \return the buffer currently shown in this window
	Buffer * buffer();
	Buffer const * buffer() const;

	///
	void openLayoutList();
	///
	bool isToolbarVisible(std::string const & id);
	///
	virtual void showMiniBuffer(bool visible) = 0;
	virtual void openMenu(docstring const & name) = 0;

	/// get access to the dialogs
	Dialogs & getDialogs() { return *dialogs_; }
	///
	Dialogs const & getDialogs() const { return *dialogs_; }

	//@}

	/// load a buffer into the current workarea.
	Buffer * loadLyXFile(support::FileName const &  name, ///< File to load.
		bool tolastfiles = true);  ///< append to the "Open recent" menu?

	/// set a buffer to the current workarea.
	void setBuffer(Buffer * b); ///< \c Buffer to set.

	/// updates the possible layouts selectable
	void updateLayoutChoice();

	/// update the toolbar
	void updateToolbars();
	/// get toolbar info
	ToolbarInfo * getToolbarInfo(std::string const & name);
	/// toggle toolbar state
	void toggleToolbarState(std::string const & name, bool allowauto);
	/// update the status bar
	virtual void updateStatusBar() = 0;

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
	Buffer const * updateInset(Inset const *);

	/// returns true if this view has the focus.
	virtual bool hasFocus() const = 0;

	/// show the error list to the user
	void showErrorList(std::string const &);

protected:
	/// connect to signals in the given BufferView
	void connectBufferView(BufferView & bv);
	/// disconnect from signals in the given BufferView
	void disconnectBufferView();
	/// connect to signals in the given buffer
	void connectBuffer(Buffer & buf);
	/// disconnect from signals in the given buffer
	void disconnectBuffer();

	/// view's toolbar
	Toolbars * toolbars_;

private:
	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(docstring const & t, docstring const & it) = 0;

	/// called on timeout
	void autoSave();

	/// auto-saving of buffers
	Timeout * const autosave_timeout_;
	/// dialogs for this view
	Dialogs * dialogs_;

	/// buffer structure changed signal connection
	boost::signals::connection bufferStructureChangedConnection_;
	/// embedded file change signal connection
	boost::signals::connection bufferEmbeddingChangedConnection_;
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
		std::string const & data, Inset * inset);
	void updateDialog(std::string const & name,
		std::string const & data);
	//@}

	/// notify readonly status
	void showReadonly(bool);

protected:
	///
	void updateToc();
	///
	void updateEmbeddedFiles();

private:
	int id_;
};

} // namespace frontend
} // namespace lyx

#endif // LYXVIEW_H
