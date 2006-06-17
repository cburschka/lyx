/**
 * \file QtView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#undef QT3_SUPPORT

#include <config.h>

#include "BufferView.h"
#include "lyx_cb.h"
#include "lyxrc.h"
#include "lyx_main.h"
#include "session.h"
#include "lyxfunc.h"
#include "MenuBackend.h"
#include "funcrequest.h"
#include "funcrequest.h"

#include "debug.h"

#include "frontends/Toolbars.h"

#include "support/filetools.h"

#include "support/convert.h"
#include <boost/bind.hpp>

#include "QtView.h"
#include "QLMenubar.h"
#include "FontLoader.h"
#include "QCommandBuffer.h"
#include "qt_helpers.h"

#include <QApplication>
#include <QPixmap>
#include <QStatusBar>
#include <QToolBar>
#include <QCloseEvent>
#include <QAction>
//#include <QMenu>
//#include <QMenuBar>

#include "support/lstrings.h"


using std::string;
using std::endl;

FontLoader fontloader;

namespace lyx {

using support::subst;
using support::libFileSearch;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon


QtView::QtView(unsigned int width, unsigned int height)
	: QMainWindow(), LyXView(), commandbuffer_(0)
{
	mainWidget_ = this;

//	setToolButtonStyle(Qt::ToolButtonIconOnly);
//	setIconSize(QSize(12,12));

	bufferview_.reset(new BufferView(this, width, height));

	menubar_.reset(new QLMenubar(this, menubackend));
	connect(menuBar(), SIGNAL(triggered(QAction *)), this, SLOT(updateMenu(QAction *)));

	getToolbars().init();

	statusBar()->setSizeGripEnabled(false);

	view_state_changed.connect(boost::bind(&QtView::update_view_state, this));
	connect(&statusbar_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));

#ifndef Q_WS_MACX
	//  assign an icon to main form. We do not do it under Qt/Mac,
	//  since the icon is provided in the application bundle.
	string const iconname = libFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setWindowIcon(QPixmap(toqstr(iconname)));
#endif

	// make sure the buttons are disabled if needed
	updateToolbars();
}


QtView::~QtView()
{
}

void QtView::updateMenu(QAction *action)
{
	menubar_->update();
}

void QtView::setWindowTitle(string const & t, string const & it)
{
	QMainWindow::setWindowTitle(toqstr(t));
	QMainWindow::setWindowIconText(toqstr(it));
}


void QtView::addCommandBuffer(QToolBar * toolbar)
{
	commandbuffer_ = new QCommandBuffer(this, *controlcommand_);
	focus_command_buffer.connect(boost::bind(&QtView::focus_command_widget, this));
	toolbar->addWidget(commandbuffer_);
}


void QtView::message(string const & str)
{
	statusBar()->showMessage(toqstr(str));
	statusbar_timer_.stop();
	statusbar_timer_.start(statusbar_timer_value);
}


void QtView::clearMessage()
{
	update_view_state_qt();
}


void QtView::focus_command_widget()
{
	if (commandbuffer_)
		commandbuffer_->focus_command();
}


void QtView::update_view_state_qt()
{
	statusBar()->showMessage(toqstr(getLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void QtView::update_view_state()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->showMessage(toqstr(getLyXFunc().viewStatusMessage()));
}


void QtView::activated(FuncRequest const & func)
{
	getLyXFunc().dispatch(func);
}


bool QtView::hasFocus() const
{
	return qApp->activeWindow() == this;
}


void QtView::closeEvent(QCloseEvent *)
{
	QRect geometry = normalGeometry();
	Session & session = LyX::ref().session();
	// save windows size and position
	session.saveSessionInfo("WindowWidth", convert<string>(geometry.width()));
	session.saveSessionInfo("WindowHeight", convert<string>(geometry.height()));
	session.saveSessionInfo("WindowIsMaximized", (isMaximized() ? "yes" : "no"));
	if (lyxrc.geometry_xysaved) {
		session.saveSessionInfo("WindowPosX", convert<string>(geometry.x()));
		session.saveSessionInfo("WindowPosY", convert<string>(geometry.y()));
	}
	// trigger LFUN_LYX_QUIT instead of quit directly
	// since LFUN_LYX_QUIT may have more cleanup stuff
	getLyXFunc().dispatch(FuncRequest(LFUN_LYX_QUIT));
}


void QtView::show()
{
	QMainWindow::setWindowTitle(qt_("LyX"));
	QMainWindow::show();
}


void QtView::busy(bool yes) const
{
	if (yes)
		QApplication::setOverrideCursor(Qt::WaitCursor);
	else
		QApplication::restoreOverrideCursor();
}

QMainWindow* QtView::mainWidget()
{
	return mainWidget_;
}

QMainWindow* QtView::mainWidget_ = 0;


} // namespace frontend
} // namespace lyx

#include "QtView_moc.cpp"
