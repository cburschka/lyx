/**
 * \file GuiView.C
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

#include "GuiView.h"
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


GuiView::GuiView(unsigned int width, unsigned int height)
	: QMainWindow(), LyXView(), commandbuffer_(0), frontend_(*this)
{
	mainWidget_ = this;

//	setToolButtonStyle(Qt::ToolButtonIconOnly);
//	setIconSize(QSize(12,12));

	bufferview_.reset(new BufferView(this, width, height));

	menubar_.reset(new QLMenubar(this, menubackend));
	connect(menuBar(), SIGNAL(triggered(QAction *)), this, SLOT(updateMenu(QAction *)));

	getToolbars().init();

	statusBar()->setSizeGripEnabled(false);

	view_state_changed.connect(boost::bind(&GuiView::update_view_state, this));
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


GuiView::~GuiView()
{
}

void GuiView::updateMenu(QAction *action)
{
	menubar_->update();
}

void GuiView::setWindowTitle(string const & t, string const & it)
{
	QMainWindow::setWindowTitle(toqstr(t));
	QMainWindow::setWindowIconText(toqstr(it));
}


void GuiView::addCommandBuffer(QToolBar * toolbar)
{
	commandbuffer_ = new QCommandBuffer(this, *controlcommand_);
	focus_command_buffer.connect(boost::bind(&GuiView::focus_command_widget, this));
	toolbar->addWidget(commandbuffer_);
}


void GuiView::message(string const & str)
{
	statusBar()->showMessage(toqstr(str));
	statusbar_timer_.stop();
	statusbar_timer_.start(statusbar_timer_value);
}


void GuiView::clearMessage()
{
	update_view_state_qt();
}


void GuiView::focus_command_widget()
{
	if (commandbuffer_)
		commandbuffer_->focus_command();
}


void GuiView::update_view_state_qt()
{
	statusBar()->showMessage(toqstr(getLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void GuiView::update_view_state()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->showMessage(toqstr(getLyXFunc().viewStatusMessage()));
}


void GuiView::activated(FuncRequest const & func)
{
	getLyXFunc().dispatch(func);
}


bool GuiView::hasFocus() const
{
	return qApp->activeWindow() == this;
}


void GuiView::closeEvent(QCloseEvent *)
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


void GuiView::show()
{
	QMainWindow::setWindowTitle(qt_("LyX"));
	QMainWindow::show();
}


void GuiView::busy(bool yes) const
{
	if (yes)
		QApplication::setOverrideCursor(Qt::WaitCursor);
	else
		QApplication::restoreOverrideCursor();
}

QMainWindow* GuiView::mainWidget()
{
	return mainWidget_;
}

QMainWindow* GuiView::mainWidget_ = 0;


} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
