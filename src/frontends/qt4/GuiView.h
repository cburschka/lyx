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
#include "FuncRequest.h"

#include <QAction>
#include <QCloseEvent>
#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>

class QDragEnterEvent;
class QDropEvent;
class QMenu;
class QToolBar;

namespace lyx {
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
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		int maximized,
		unsigned int iconSizeXY,
		const std::string & geometryArg);
	virtual void saveGeometry();
	virtual void setBusy(bool);
	/// add toolbar, if newline==true, add a toolbar break before the toolbar
	GuiToolbar * makeToolbar(ToolbarInfo const & tbinfo, bool newline);
	virtual void updateStatusBar();
	virtual void message(docstring const & str);
	virtual void clearMessage();
	virtual bool hasFocus() const;
	void showMiniBuffer(bool);
	void openMenu(docstring const &);
	void openLayoutList();
	void updateLayoutChoice(bool force);
	bool isToolbarVisible(std::string const & id);
	void updateToolbars();
	ToolbarInfo * getToolbarInfo(std::string const & name);
	void toggleToolbarState(std::string const & name, bool allowauto);

	/// show - display the top-level window
	void showView();

	/// menu item has been selected
	void activated(FuncRequest const &);

	QMenu* createPopupMenu();

Q_SIGNALS:
	void closing(int);

public Q_SLOTS:
	/// idle timeout
	void update_view_state_qt();

	///
	void on_currentWorkAreaChanged(GuiWorkArea *);

	/// slots to change the icon size
	void smallSizedIcons();
	void normalSizedIcons();
	void bigSizedIcons();

protected:
	/// make sure we quit cleanly
	virtual void closeEvent(QCloseEvent * e);
	///
	virtual void resizeEvent(QResizeEvent * e);
	///
	virtual void moveEvent(QMoveEvent * e);

	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	WorkArea * workArea(Buffer & buffer);

	/// Add a \c WorkArea 
	/// \return the \c Workarea associated to \p  Buffer
	/// \retval 0 if no \c WorkArea is found.
	WorkArea * addWorkArea(Buffer & buffer);
	void setCurrentWorkArea(WorkArea * work_area);
	void removeWorkArea(WorkArea * work_area);
	WorkArea const * currentWorkArea() const;
	WorkArea * currentWorkArea();

private:
	///
	void dragEnterEvent(QDragEnterEvent * ev);
	///
	void dropEvent(QDropEvent * ev);

	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(docstring const & t, docstring const & it);

	/// in order to catch Tab key press.
	bool event(QEvent * e);
	bool focusNextPrevChild(bool);

	QTimer statusbar_timer_;

	/// are we quitting by the menu?
	bool quitting_by_menu_;

	///
	QRect updateFloatingGeometry();
	///
	QRect floatingGeometry_;

	void setIconSize(unsigned int size);

	struct ToolbarSize {
		int top_width;
		int bottom_width;
		int left_height;
		int right_height;
	};

	ToolbarSize toolbarSize_;

	struct GuiViewPrivate;
	GuiViewPrivate& d;
};


} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
