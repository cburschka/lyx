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
 */
class GuiView : public QMainWindow, public LyXView {
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	GuiView();

	~GuiView();

	/// initialize the object
	virtual void init();

	/// show - display the top-level window
	void show();

	/// show busy cursor
	virtual void busy(bool) const;

	Toolbars::ToolbarPtr makeToolbar(ToolbarBackend::Toolbar const & tbb);

	/// display a status message
	virtual void message(lyx::docstring const & str);

	/// clear status message
	virtual void clearMessage();

	/// update the status bar
	virtual void updateStatusBar();

	/// add the command buffer
	void addCommandBuffer(QToolBar * toolbar);

	/// menu item has been selected
	void activated(FuncRequest const &);

	/// returns true if this view has the focus.
	virtual bool hasFocus() const;

public Q_SLOTS:
	/// idle timeout
	void update_view_state_qt();

	/// populate a toplevel menu and all its children on demand
	void updateMenu(QAction *);

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
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
