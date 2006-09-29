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

#include "frontends/WorkArea.h"
#include "support/filetools.h"
#include "support/convert.h"
#include "support/lstrings.h"

// This include must be declared before everything else because
// of boost/Qt/LyX clash...
#include "GuiImplementation.h"

#include "GuiView.h"
#include "QLMenubar.h"
#include "QLToolbar.h"
#include "FontLoader.h"
#include "QCommandBuffer.h"
#include "qt_helpers.h"

#include <QApplication>
#include <QPixmap>
#include <QStatusBar>
#include <QToolBar>
#include <QCloseEvent>
#include <QAction>


#include <boost/bind.hpp>


using std::string;
using std::endl;

namespace lyx {

using support::subst;
using support::libFileSearch;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon


GuiView::GuiView()
	: QMainWindow(), LyXView(), commandbuffer_(0)
{
//	setToolButtonStyle(Qt::ToolButtonIconOnly);
//	setIconSize(QSize(12,12));

//	bufferview_.reset(new BufferView(this, width, height));

#ifndef Q_WS_MACX
	//  assign an icon to main form. We do not do it under Qt/Mac,
	//  since the icon is provided in the application bundle.
	string const iconname = libFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setWindowIcon(QPixmap(toqstr(iconname)));
#endif
}


GuiView::~GuiView()
{
}


void GuiView::init()
{
	menubar_.reset(new QLMenubar(this, menubackend));
	QObject::connect(menuBar(), SIGNAL(triggered(QAction *)), this, SLOT(updateMenu(QAction *)));

	getToolbars().init();

	statusBar()->setSizeGripEnabled(false);

	QObject::connect(&statusbar_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));

	// make sure the buttons are disabled if needed
	updateToolbars();
	updateLayoutChoice();
	updateMenubar();
}


void GuiView::setGeometry(unsigned int width,
								  unsigned int height,
								  int posx, int posy,
								  bool maximize)
{
	// only true when the -geometry option was NOT used
	if (width != 0 && height != 0) {
		if (posx != -1 && posy != -1) {
#ifdef Q_OS_WIN32
			// FIXME: use only setGeoemtry when Trolltech has
			// fixed the qt4/X11 bug
			QMainWindow::setGeometry(posx, posy,width, height);
#else
			resize(width, height);
			move(posx, posy);
#endif
		} else {
			resize(width, height);
		}

		if (maximize)
			setWindowState(Qt::WindowMaximized);
	}
	
	show();
}


void GuiView::updateMenu(QAction * /*action*/)
{
	menubar_->update();
}


void GuiView::setWindowTitle(docstring const & t, docstring const & it)
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


void GuiView::message(docstring const & str)
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


void GuiView::updateStatusBar()
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


void  GuiView::updateFloatingGeometry()
{
	if (!isMaximized())
		floatingGeometry_ = QRect(x(), y(), width(), height());
}


void GuiView::resizeEvent(QResizeEvent *)
{
	updateFloatingGeometry();
}


void GuiView::moveEvent(QMoveEvent *)
{
	updateFloatingGeometry();
}


void GuiView::closeEvent(QCloseEvent *)
{
	// FIXME:
	// change the ifdef to 'geometry = normalGeometry();' only
	// when Trolltech has fixed the broken normalGeometry on X11:
	// http://www.trolltech.com/developer/task-tracker/index_html?id=119684+&method=entry
	// Then also the moveEvent, resizeEvent, and the
	// code for floatingGeometry_ can be removed;
	// adjust lyx_gui::start
#ifdef Q_OS_WIN32
	QRect geometry = normalGeometry();
#else
	updateFloatingGeometry();
	QRect geometry = floatingGeometry_;
#endif

	// save windows size and position
	Session & session = LyX::ref().session();
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
	updateFloatingGeometry();
}


void GuiView::busy(bool yes) const
{
	if (yes)
		QApplication::setOverrideCursor(Qt::WaitCursor);
	else
		QApplication::restoreOverrideCursor();
}


Toolbars::ToolbarPtr GuiView::makeToolbar(ToolbarBackend::Toolbar const & tbb)
{
	QLToolbar * Tb = new QLToolbar(tbb, *this);
	static QLToolbar * lastTb = 0;

	if (tbb.flags & ToolbarBackend::TOP) {
			addToolBar(Qt::TopToolBarArea, Tb);
			addToolBarBreak(Qt::TopToolBarArea);
	}
	if (tbb.flags & ToolbarBackend::BOTTOM) {
		addToolBar(Qt::BottomToolBarArea, Tb);
		if (lastTb)
			insertToolBarBreak(lastTb);
		lastTb = Tb;
	}
	if (tbb.flags & ToolbarBackend::LEFT) {
		addToolBar(Qt::LeftToolBarArea, Tb);
	}
	if (tbb.flags & ToolbarBackend::RIGHT) {
		addToolBar(Qt::RightToolBarArea, Tb);
	}

	return Toolbars::ToolbarPtr(Tb);
}

} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
