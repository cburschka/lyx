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

#include <QMainWindow>
#include <QTimer>

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;


namespace lyx {

class Timeout;
class ToolbarInfo;

namespace frontend {

class GuiToolbar;
class GuiWorkArea;

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

	enum Maximized {
		NotMaximized = 0, // LyX::newLyXView() relies on this to be zero!
		VerticallyMaximized,
		HorizontallyMaximized,
		CompletelyMaximized
	};
	///
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		Maximized maximized,
		unsigned int iconSizeXY,
		const std::string & geometryArg);
	/// save the geometry state in the session manager.
	virtual void saveGeometry();
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
	void updateDialog(std::string const & name,
		std::string const & data);
	
	/// called on timeout
	void autoSave();
	///
	void updateEmbeddedFiles();

	/// \return the current buffer view.
	BufferView * view();

	/// get access to the dialogs
	Dialogs & getDialogs() { return *dialogs_; }
	///
	Dialogs const & getDialogs() const { return *dialogs_; }

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
	virtual void closeEvent(QCloseEvent * e);
	///
	virtual void resizeEvent(QResizeEvent * e);
	///
	virtual void moveEvent(QMoveEvent * e);

	/// in order to catch Tab key press.
	bool event(QEvent * e);
	bool focusNextPrevChild(bool);
	///
	QRect updateFloatingGeometry();
	///
	void setIconSize(unsigned int size);

	///
	struct GuiViewPrivate;
	GuiViewPrivate & d;

	///
	QTimer statusbar_timer_;

	/// are we quitting by the menu?
	bool quitting_by_menu_;

	///
	QRect floatingGeometry_;

	struct ToolbarSize {
		int top_width;
		int bottom_width;
		int left_height;
		int right_height;
	};

	ToolbarSize toolbarSize_;

	/// auto-saving of buffers
	Timeout * const autosave_timeout_;
	/// dialogs for this view
	Dialogs * dialogs_;
};


} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
