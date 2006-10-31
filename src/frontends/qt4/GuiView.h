// -*- C++ -*-
/**
 * \file GuiView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjornes
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_VIEW_H
#define GUI_VIEW_H

// Must be here because of moc.
#include <config.h>

#include "frontends/LyXView.h"
#include "funcrequest.h"

#include <QMainWindow>
#include <QTimer>
#include <QAction>
#include <QCloseEvent>

class QToolBar;
//class FuncRequest;

//class string;

namespace lyx {
namespace frontend {

class QCommandBuffer;

QWidget* mainWindow();

/**
 * GuiView - Qt4 implementation of LyXView
 *
 * qt4-private implementation of the main LyX window.
 *
 * Note: any QObject emits a destroyed(QObject *) Qt signal when it
 *       is deleted.This might be useful for closing other dialogs
 *       depending on a given GuiView.
 */
class GuiView : public QMainWindow, public LyXView {
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	GuiView(int id);

	~GuiView();

	virtual void init();
	virtual void close();
	virtual void setGeometry(
		unsigned int width,
		unsigned int height,
		int posx, int posy,
		bool maximize);
	virtual void saveGeometry();
	virtual void busy(bool);
	Toolbars::ToolbarPtr makeToolbar(ToolbarBackend::Toolbar const & tbb);
	virtual void updateStatusBar();
	virtual void message(lyx::docstring const & str);
	virtual void clearMessage();
	virtual bool hasFocus() const;

	virtual void updateTab();

	/// show - display the top-level window
	void show();

	/// add the command buffer
	void addCommandBuffer(QToolBar * toolbar);

	/// menu item has been selected
	void activated(FuncRequest const &);

	void initTab(QWidget* workArea);

Q_SIGNALS:
	void closing(int);

public Q_SLOTS:
	/// idle timeout
	void update_view_state_qt();

	/// populate a toplevel menu and all its children on demand
	void updateMenu(QAction *);

	void currentTabChanged (int index); 

protected:
	/// make sure we quit cleanly
	virtual void closeEvent(QCloseEvent * e);

	///
	virtual void resizeEvent(QResizeEvent * e);

	///
	virtual void moveEvent(QMoveEvent * e);

private:
	/// focus the command buffer widget
	void focus_command_widget();

	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(lyx::docstring const & t, lyx::docstring const & it);

	QTimer statusbar_timer_;

	/// command buffer
	QCommandBuffer * commandbuffer_;

	///
	void updateFloatingGeometry();
	///
	QRect floatingGeometry_;

	struct GuiViewPrivate;
	GuiViewPrivate& d;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
