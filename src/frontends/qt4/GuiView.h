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

#include "support/strfwd.h"

#include <QMainWindow>

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QMenu;
class QShowEvent;


namespace lyx {

namespace support { class FileName; }

class Buffer;
class BufferView;
class Cursor;
class DispatchResult;
class FuncStatus;
class FuncRequest;
class Inset;

namespace frontend {

class Dialog;
class LayoutBox;
class GuiToolbar;
class GuiWorkArea;
class TabWorkArea;
class TocModels;
class ToolbarInfo;

/**
 * GuiView - Qt main LyX window
 *
 * This class represents the main LyX window and provides
 * accessor functions to its content.
 *
 * Note: a QObject emits a destroyed(QObject *) Qt signal when it
 * is deleted. This might be useful for closing other dialogs
 * depending on a given GuiView.
 */
class GuiView : public QMainWindow, public GuiBufferViewDelegate,
	public GuiBufferDelegate
{
	Q_OBJECT

public:
	/// create a main window of the given dimensions
	GuiView(int id);

	~GuiView();

	/// closes the view such that the view knows that is closed
	/// programmatically and not by the user clicking the x.
	bool closeScheduled();

	/// Things that need to be done when the OSes session manager
	/// requests a log out.
	bool prepareAllBuffersForLogout();

	int id() const { return id_; }

	/// are we busy ?
	bool busy() const;

	/// Signal that the any "auto" minibuffer can be closed now.
	void resetCommandExecute();

	/// \name Generic accessor functions
	//@{
	/// The current BufferView refers to the BufferView that has the focus,
	/// including for example the one that is created when you use the
	/// advanced search and replace pane.
	/// \return the currently selected buffer view.
	BufferView * currentBufferView();
	BufferView const * currentBufferView() const;

	/// The document BufferView always refers to the view's main document
	/// BufferView. So, even if the BufferView in e.g., the advanced
	/// search and replace pane has the focus.
	/// \return the current document buffer view.
	BufferView * documentBufferView();
	BufferView const * documentBufferView() const;

	void newDocument(std::string const & filename,
		bool fromTemplate);

	/// display a message in the view
	/// could be called from any thread
	void message(docstring const &);
	
	bool getStatus(FuncRequest const & cmd, FuncStatus & flag);
	/// dispatch command.
	/// \return true if the \c FuncRequest has been dispatched.
	void dispatch(FuncRequest const & cmd, DispatchResult & dr);

	void restartCursor();
	/// Update the completion popup and the inline completion state.
	/// If \c start is true, then a new completion might be started.
	/// If \c keep is true, an active completion will be kept active
	/// even though the cursor moved. The update flags of \c cur might
	/// be changed.
	void updateCompletion(Cursor & cur, bool start, bool keep);

	///
	void setFocus();
	bool hasFocus() const;

	///
	void focusInEvent(QFocusEvent * e);
	/// set a buffer to the current workarea.
	void setBuffer(Buffer * b); ///< \c Buffer to set.

	/// load a document into the current workarea.
	Buffer * loadDocument(
		support::FileName const &  name, ///< File to load.
		bool tolastfiles = true  ///< append to the "Open recent" menu?
		);

	/// add toolbar, if newline==true, add a toolbar break before the toolbar
	GuiToolbar * makeToolbar(ToolbarInfo const & tbinfo, bool newline);
	void updateStatusBar();

	/// updates the possible layouts selectable
	void updateLayoutList();
	void updateToolbars();
	QMenu * createPopupMenu();

	///
	LayoutBox * getLayoutDialog() const;

	/// hides the workarea and makes sure it is clean
	bool hideWorkArea(GuiWorkArea * wa);
	/// closes workarea; close buffer only if no other workareas point to it
	bool closeWorkArea(GuiWorkArea * wa);
	/// closes the buffer
	bool closeBuffer(Buffer & buf);
	///
	void openDocument(std::string const & filename);
	///
	void importDocument(std::string const &);

	/// \name GuiBufferDelegate.
	//@{
	void resetAutosaveTimers();
	void errors(std::string const &, bool from_master = false);
	void structureChanged();
	void updateTocItem(std::string const &, DocIterator const &);
	//@}

	///
	TocModels & tocModels();
	
	/// called on timeout
	void autoSave();

	/// check for external change of any opened buffer, mainly for svn usage
	void checkExternallyModifiedBuffers();

	/** redraw \c inset in all the BufferViews in which it is currently
	 *  visible. If successful return a pointer to the owning Buffer.
	 */
	Buffer const * updateInset(Inset const *);

	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	GuiWorkArea * workArea(Buffer & buffer);
	/// \return the \c Workarea at index \c index
	GuiWorkArea * workArea(int index);

	/// Add a \c WorkArea 
	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	GuiWorkArea * addWorkArea(Buffer & buffer);
	/// \param work_area The current \c WorkArea, or \c NULL
	void setCurrentWorkArea(GuiWorkArea * work_area);
	///
	void removeWorkArea(GuiWorkArea * work_area);
	/// return the current WorkArea (the one that has the focus).
	GuiWorkArea const * currentWorkArea() const;
	/// return the current WorkArea (the one that has the focus).
	GuiWorkArea * currentWorkArea();

	/// return the current document WorkArea (it may not have the focus).
	GuiWorkArea const * currentMainWorkArea() const;
	/// return the current document WorkArea (it may not have the focus).
	GuiWorkArea * currentMainWorkArea();
	
	/// Current ratio between physical pixels and device-independent pixels
	double pixelRatio() const;

Q_SIGNALS:
	void closing(int);
	void triggerShowDialog(QString const & qname, QString const & qdata, Inset * inset);

public Q_SLOTS:
	///
	void setBusy(bool);
	/// idle timeout.
	/// clear any temporary message and replace with current status.
	void clearMessage();
	///
	void updateWindowTitle(GuiWorkArea * wa);

private Q_SLOTS:
	///
	void resetWindowTitle();

	///
	void on_currentWorkAreaChanged(GuiWorkArea *);
	///
	void on_lastWorkAreaRemoved();

	/// slots to change the icon size
	void smallSizedIcons();
	void normalSizedIcons();
	void bigSizedIcons();
	void hugeSizedIcons();
	void giantSizedIcons();

	/// For completion of autosave or export threads.
	void processingThreadStarted();
	void processingThreadFinished();
	void autoSaveThreadFinished();

	/// must be called in GUI thread
	void doShowDialog(QString const & qname, QString const & qdata,
	Inset * inset);

	/// must be called from GUI thread
	void updateStatusBarMessage(QString const & str);
	void clearMessageText();

private:
	/// Open given child document in current buffer directory.
	void openChildDocument(std::string const & filename);
	/// Close current document buffer.
	bool closeBuffer();
	/// Close all document buffers.
	bool closeBufferAll();
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
	bool goToFileRow(std::string const & argument);

	///
	class GuiViewPrivate;
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

	/** Show dialog could be called from arbitrary threads.
	    \param name == "bibtex", "citation" etc; an identifier used to
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

private:
	/// Saves the layout and geometry of the window
	void saveLayout() const;
	/// Saves the settings of toolbars and all dialogs
	void saveUISettings() const;
	///
	bool restoreLayout();
	///
	GuiToolbar * toolbar(std::string const & name);
	///
	void constructToolbars();
	///
	void initToolbars();
	///
	void initToolbar(std::string const & name);
	///
	bool lfunUiToggle(std::string const & ui_component);
	///
	void toggleFullScreen();
	///
	void insertLyXFile(docstring const & fname);
	///
	/// Open Export As ... dialog. \p iformat is the format the
	/// filter is initially set to.
	bool exportBufferAs(Buffer & b, docstring const & iformat);

	///
	enum RenameKind { LV_WRITE_AS, LV_VC_RENAME, LV_VC_COPY };
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

	 \p kind controls what is done besides the pure renaming:
         * LV_WRITE_AS  => The buffer is written without version control actions.
         * LV_VC_RENAME => The file is renamed in version control.
         * LV_VC_COPY   => The file is copied in version control.
	 */
	bool renameBuffer(Buffer & b, docstring const & newname,
	                  RenameKind kind = LV_WRITE_AS);
	///
	bool saveBuffer(Buffer & b);
	/// save and rename buffer to fn. If fn is empty, the buffer
	/// is just saved as the filename it already has.
	bool saveBuffer(Buffer & b, support::FileName const & fn);
	/// closes a workarea, if close_buffer is true the buffer will
	/// also be released, otherwise the buffer will be hidden.
	bool closeWorkArea(GuiWorkArea * wa, bool close_buffer);
	/// closes the tabworkarea and all tabs. If we are in a close event,
	/// all buffers will be closed, otherwise they will be hidden.
	bool closeTabWorkArea(TabWorkArea * twa);
	/// gives the user the possibility to save his work 
	/// or to discard the changes. If hiding is true, the
	/// document will be reloaded.
	bool saveBufferIfNeeded(Buffer & buf, bool hiding);
	/// closes all workareas
	bool closeWorkAreaAll();
	/// write all open workareas into the session file
	void writeSession() const;
	/// is the buffer in this workarea also shown in another tab ?
	/// This tab can either be in the same view or in another one.
	bool inMultiTabs(GuiWorkArea * wa);
	/// is the buffer shown in some other view ?
	bool inOtherView(Buffer & buf);
	///
	enum NextOrPrevious {
		NEXTBUFFER,
		PREVBUFFER
	};
	///
	void gotoNextOrPreviousBuffer(NextOrPrevious np, bool const move);

	/// Is the dialog currently visible?
	bool isDialogVisible(std::string const & name) const;
	///
	Dialog * findOrBuild(std::string const & name, bool hide_it);
	///
	Dialog * build(std::string const & name);
	///
	bool reloadBuffer(Buffer & buffer);
	///
	void dispatchVC(FuncRequest const & cmd, DispatchResult & dr);
	///
	void dispatchToBufferView(FuncRequest const & cmd, DispatchResult & dr);
	///
	void showMessage();

	/// This view ID.
	int id_;

	/// flag to avoid two concurrent close events.
	bool closing_;
	/// if the view is busy the cursor shouldn't blink for instance.
	/// This counts the number of times more often we called
	/// setBusy(true) compared to setBusy(false), so we can nest
	/// functions that call setBusy;
	int busy_;

	/// Request to open the command toolbar if it is "auto"
	bool command_execute_;
	/// Request to give focus to minibuffer
	bool minibuffer_focus_;

	/// Statusbar widget that shows read-only status
	QLabel * read_only_;
	/// Statusbar widget that shows version control status
	QLabel * version_control_;

	/// Minimum zoom percentage
	unsigned int const zoom_min = 10;

};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
