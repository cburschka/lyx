// -*- C++ -*-
/**
 * \file GuiView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjornes
 * \author John Levon
 * \author Abdelrazak Younes
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_VIEW_H
#define GUI_VIEW_H

#include "frontends/LyXView.h"

#include <string>


#include <QMainWindow>
#include <QTimer>

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QShowEvent;


namespace lyx {

class Timeout;
class ToolbarInfo;

namespace frontend {

class GuiToolbar;
class GuiWorkArea;
class Dialog;

QWidget * mainWindow();

/**
 * GuiView - Qt4 implementation of LyXView
 *
 * qt4-private implementation of the main LyX window.
 *
 * Note: a QObject emits a destroyed(QObject *) Qt signal when it
 * is deleted. This might be useful for closing other dialogs
 * depending on a given GuiView.
 */
class GuiView : public QMainWindow, public LyXView
{
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	GuiView(int id);

	~GuiView();

	virtual void init();
	virtual void close();
	virtual void setFocus();
	virtual void setBusy(bool);

	/// add toolbar, if newline==true, add a toolbar break before the toolbar
	GuiToolbar * makeToolbar(ToolbarInfo const & tbinfo, bool newline);
	virtual void updateStatusBar();
	virtual void message(docstring const & str);
	virtual bool hasFocus() const;
	void updateLayoutChoice(bool force);
	bool isToolbarVisible(std::string const & id);
	void updateToolbars();
	///
	QMenu * createPopupMenu();

	/// dispatch to current BufferView
	void dispatch(FuncRequest const & cmd);

	/// \return the buffer currently shown in this window
	Buffer * buffer();
	Buffer const * buffer() const;
	/// set a buffer to the current workarea.
	void setBuffer(Buffer * b); ///< \c Buffer to set.

	/// GuiBufferDelegate.
	///@{
	void resetAutosaveTimers();
	void errors(std::string const &);
	void structureChanged() { updateToc(); }
	///@}

	////
	void showDialog(std::string const & name);
	void showDialogWithData(std::string const & name,
		std::string const & data);
	void showInsetDialog(std::string const & name,
		std::string const & data, Inset * inset);
	
	/// called on timeout
	void autoSave();
	///
	void updateEmbeddedFiles();

	/// \return the current buffer view.
	BufferView * view();

	/// load a buffer into the current workarea.
	Buffer * loadLyXFile(support::FileName const &  name, ///< File to load.
		bool tolastfiles = true);  ///< append to the "Open recent" menu?

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	Buffer const * updateInset(Inset const *);
	///
	void restartCursor();

	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	GuiWorkArea * workArea(Buffer & buffer);

	/// Add a \c WorkArea 
	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	GuiWorkArea * addWorkArea(Buffer & buffer);
	///
	void setCurrentWorkArea(GuiWorkArea * work_area);
	///
	void removeWorkArea(GuiWorkArea * work_area);
	/// return the current WorkArea (the one that has the focus).
	GuiWorkArea const * currentWorkArea() const;

Q_SIGNALS:
	void closing(int);

public Q_SLOTS:
	/// idle timeout.
	/// clear any temporary message and replace with current status.
	void clearMessage();

private Q_SLOTS:
	///
	void updateWindowTitle(GuiWorkArea * wa);

	///
	void on_currentWorkAreaChanged(GuiWorkArea *);

	/// slots to change the icon size
	void smallSizedIcons();
	void normalSizedIcons();
	void bigSizedIcons();

private:
	///
	void addTabWorkArea();

	/// connect to signals in the given BufferView
	void connectBufferView(BufferView & bv);
	/// disconnect from signals in the given BufferView
	void disconnectBufferView();
	/// connect to signals in the given buffer
	void connectBuffer(Buffer & buf);
	/// disconnect from signals in the given buffer
	void disconnectBuffer();
	///
	void updateToc();
	///
	void dragEnterEvent(QDragEnterEvent * ev);
	///
	void dropEvent(QDropEvent * ev);
	/// make sure we quit cleanly
	void closeEvent(QCloseEvent * e);
	///
	void showEvent(QShowEvent *);

	/// in order to catch Tab key press.
	bool event(QEvent * e);
	bool focusNextPrevChild(bool);

	///
	struct GuiViewPrivate;
	GuiViewPrivate & d;

	///
	QTimer statusbar_timer_;

	/// are we quitting by the menu?
	bool quitting_by_menu_;

	struct ToolbarSize {
		int top_width;
		int bottom_width;
		int left_height;
		int right_height;
	};

	ToolbarSize toolbarSize_;

	/// auto-saving of buffers
	Timeout * const autosave_timeout_;

public:
	///
	/// dialogs for this view
	///

	/** Check the status of all visible dialogs and disable or reenable
	 *  them as appropriate.
	 *
	 *  Disabling is needed for example when a dialog is open and the
	 *  cursor moves to a position where the corresponding inset is not
	 *  allowed.
	 */
	void checkStatus();

	/// Are the tooltips on or off?
	bool tooltipsEnabled();

	/// Hide all visible dialogs
	void hideAll() const;
	/// Hide any dialogs that require a buffer for them to operate
	void hideBufferDependent() const;
	/** Update visible, buffer-dependent dialogs
	    If the bool is true then a buffer change has occurred
	    else it is still the same buffer.
	 */
	void updateBufferDependent(bool) const;

	/** \param name == "bibtex", "citation" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string representation of the Inset contents.
	    It is often little more than the output from Inset::write.
	    It is passed to, and parsed by, the frontend dialog.
	    Several of these dialogs do not need any data,
	    so it defaults to string().
	    \param inset ownership is _not_ passed to the frontend dialog.
	    It is stored internally and used by the kernel to ascertain
	    what to do with the FuncRequest dispatched from the frontend
	    dialog on 'Apply'; should it be used to create a new inset at
	    the current cursor position or modify an existing, 'open' inset?
	*/
	void showDialog(std::string const & name,
		std::string const & data = std::string(), Inset * inset = 0);

	/** \param name == "citation", "bibtex" etc; an identifier used
	    to update the contents of a particular dialog with \param data.
	    See the comments to 'show', above.
	*/
	void updateDialog(std::string const & name, std::string const & data);

	/// Is the dialog currently visible?
	bool isDialogVisible(std::string const & name) const;

	/** All Dialogs of the given \param name will be closed if they are
	    connected to the given \param inset.
	*/
	void hideDialog(std::string const & name, Inset * inset);
	///
	void disconnectDialog(std::string const & name);
	///
	Inset * getOpenInset(std::string const & name) const;

private:
	///
	void redrawDialog() const;
	///
	bool isValidName(std::string const & name) const;
	///
	Dialog * find_or_build(std::string const & name);
	///
	Dialog * build(std::string const & name);

	///
	/// flag against a race condition due to multiclicks in Qt frontend,
	/// see bug #1119
	bool in_show_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
