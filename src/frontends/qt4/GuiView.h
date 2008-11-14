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

#include "frontends/Delegates.h"
#include "frontends/LyXView.h"

#include "support/strfwd.h"

#include <QMainWindow>

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QShowEvent;


namespace lyx {

class Cursor;

namespace frontend {

class Dialog;
class GuiLayoutBox;
class GuiToolbar;
class GuiWorkArea;
class TabWorkArea;
class TocModels;
class ToolbarInfo;

/**
 * GuiView - Qt4 implementation of LyXView
 *
 * qt4-private implementation of the main LyX window.
 *
 * Note: a QObject emits a destroyed(QObject *) Qt signal when it
 * is deleted. This might be useful for closing other dialogs
 * depending on a given GuiView.
 */
class GuiView : public QMainWindow, public LyXView, public GuiBufferViewDelegate,
	public GuiBufferDelegate
{
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	GuiView(int id);

	~GuiView();

	///
	int id() const { return id_; }
	void setFocus();
	void setBusy(bool);
	/// returns true if this view has the focus.
	bool hasFocus() const;

	/// add toolbar, if newline==true, add a toolbar break before the toolbar
	GuiToolbar * makeToolbar(ToolbarInfo const & tbinfo, bool newline);
	virtual void updateStatusBar();
	virtual void message(docstring const & str);

	/// updates the possible layouts selectable
	void updateLayoutList();
	void updateToolbars();
	QMenu * createPopupMenu();
	bool getStatus(FuncRequest const & cmd, FuncStatus & flag);
	bool dispatch(FuncRequest const & cmd);

	///
	void setLayoutDialog(GuiLayoutBox *);

	/// \return the buffer currently shown in this window
	Buffer * buffer();
	Buffer const * buffer() const;
	/// set a buffer to the current workarea.
	void setBuffer(Buffer * b); ///< \c Buffer to set.
	///
	bool closeBuffer();
	/// load a document into the current workarea.
	Buffer * loadDocument(support::FileName const &  name, ///< File to load.
		bool tolastfiles = true);  ///< append to the "Open recent" menu?
	///
	void openDocument(std::string const & filename);
	///
	void importDocument(std::string const &);
	///
	void newDocument(std::string const & filename, bool fromTemplate);

	/// GuiBufferDelegate.
	///@{
	void resetAutosaveTimers();
	void errors(std::string const &);
	void structureChanged();
	void updateTocItem(std::string const &, DocIterator const &);
	///@}

	///
	TocModels & tocModels();
	
	/// called on timeout
	void autoSave();

	/// \return the current buffer view.
	BufferView * view();

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
	void resetWindowTitleAndIconText();

	///
	void on_currentWorkAreaChanged(GuiWorkArea *);
	///
	void on_lastWorkAreaRemoved();

	/// slots to change the icon size
	void smallSizedIcons();
	void normalSizedIcons();
	void bigSizedIcons();

private:
	///
	TabWorkArea * addTabWorkArea();

	/// connect to signals in the given BufferView
	void connectBufferView(BufferView & bv);
	/// disconnect from signals in the given BufferView
	void disconnectBufferView();
	/// connect to signals in the given buffer
	void connectBuffer(Buffer & buf);
	/// disconnect from signals in the given buffer
	void disconnectBuffer();
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

public:
	///
	/// dialogs for this view
	///

	///
	void resetDialogs();

	/// Hide all visible dialogs
	void hideAll() const;

	/// Update all visible dialogs.
	/** 
	 *  Check the status of all visible dialogs and disable or reenable
	 *  them as appropriate.
	 *
	 *  Disabling is needed for example when a dialog is open and the
	 *  cursor moves to a position where the corresponding inset is not
	 *  allowed.
	 */
	void updateDialogs();

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
		std::string const & data, Inset * inset = 0);

	/** \param name == "citation", "bibtex" etc; an identifier used
	    to reset the contents of a particular dialog with \param data.
	    See the comments to 'show', above.
	*/
	void updateDialog(std::string const & name, std::string const & data);

	/** All Dialogs of the given \param name will be closed if they are
	    connected to the given \param inset.
	*/
	void hideDialog(std::string const & name, Inset * inset);
	///
	void disconnectDialog(std::string const & name);

	///
	void updateCompletion(Cursor & cur, bool start, bool keep);

private:
	///
	void saveLayout() const;
	///
	bool restoreLayout();
	///
	GuiToolbar * toolbar(std::string const & name);
	///
	void constructToolbars();
	///
	void initToolbars();
	///
	void lfunUiToggle(FuncRequest const & cmd);
	///
	void toggleFullScreen();
	///
	void insertLyXFile(docstring const & fname);
	///
	void insertPlaintextFile(docstring const & fname,
		bool asParagraph);

	/// Save a buffer as a new file. 
	/**
	Write a buffer to a new file name and rename the buffer
    according to the new file name.

    This function is e.g. used by menu callbacks and
    LFUN_BUFFER_WRITE_AS.

    If 'newname' is empty, the user is asked via a
    dialog for the buffer's new name and location.

    If 'newname' is non-empty and has an absolute path, that is used.
    Otherwise the base directory of the buffer is used as the base
    for any relative path in 'newname'.
	*/
	bool renameBuffer(Buffer & b, docstring const & newname);
	///
	bool saveBuffer(Buffer & b);
	///
	bool closeBuffer(Buffer & buf, bool tolastopened = false);

	///
	Inset * getOpenInset(std::string const & name) const;

	/// Is the dialog currently visible?
	bool isDialogVisible(std::string const & name) const;
	///
	Dialog * findOrBuild(std::string const & name, bool hide_it);
	///
	Dialog * build(std::string const & name);

	/// This view ID.
	int id_;

	/// flag to avoid two concurrent close events.
	bool closing_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
