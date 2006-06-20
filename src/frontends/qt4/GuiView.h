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

#ifndef GUIVIEW_H
#define GUIVIEW_H

// Must be here because of moc.
#include <config.h>

#include "GuiImplementation.h"

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
 * Qt-private implementation of the main LyX window.
 */
class GuiView : public QMainWindow, public LyXView {
	Q_OBJECT
public:
	/// create a main window of the given dimensions
	GuiView(unsigned int w, unsigned int h);

	~GuiView();

	/// show - display the top-level window
	void show();

	/// show busy cursor
	virtual void busy(bool) const;

	/// display a status message
	virtual void message(std::string const & str);

	/// clear status message
	virtual void clearMessage();

	/// add the command buffer
	void addCommandBuffer(QToolBar * toolbar);

	/// menu item has been selected
	void activated(FuncRequest const &);

	// returns true if this view has the focus.
	virtual bool hasFocus() const;

	//
	Gui & gui() { return frontend_; }

	static QMainWindow* mainWidget();

public slots:
	/// idle timeout
	void update_view_state_qt();

	/// populate a toplevel menu and all its children on demand
	void updateMenu(QAction *);

protected:
	/// make sure we quit cleanly
	virtual void closeEvent(QCloseEvent * e);
private:
	/// focus the command buffer widget
	void focus_command_widget();

	/// update status bar
	void update_view_state();

	/**
	 * setWindowTitle - set title of window
	 * @param t main window title
	 * @param it iconified (short) title
	 */
	virtual void setWindowTitle(std::string const & t, std::string const & it);

	QTimer statusbar_timer_;

	/// command buffer
	QCommandBuffer * commandbuffer_;

	///
	static QMainWindow* mainWidget_;

	GuiImplementation frontend_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIVIEW_H
