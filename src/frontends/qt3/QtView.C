/**
 * \file QtView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
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

#include "frontends/Toolbars.h"

#include "support/filetools.h"
#include "support/convert.h"

#include <boost/bind.hpp>

#include "QtView.h"
#include "QLMenubar.h"
#include "qfont_loader.h"
#include "QCommandBuffer.h"
#include "qt_helpers.h"

#include <qapplication.h>
#include <qpixmap.h>
#include <qstatusbar.h>

using std::string;

FontLoader fontloader;

namespace lyx {

using support::libFileSearch;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon


QtView::QtView(Gui & owner)
	: QMainWindow(), LyXView(owner), commandbuffer_(0)
{
	qApp->setMainWidget(this);

#ifndef Q_WS_MACX
	//  assign an icon to main form. We do not do it under Qt/Mac,
	//  since the icon is provided in the application bundle.
	string const iconname = libFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setIcon(QPixmap(toqstr(iconname)));
#endif
}


QtView::~QtView()
{
}


void QtView::init()
{
	menubar_.reset(new QLMenubar(this, menubackend));
	getToolbars().init();

	statusBar()->setSizeGripEnabled(false);

	connect(&statusbar_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));

	// make sure the buttons are disabled if needed
	updateToolbars();

	// allowing the toolbars to tear off is too easily done,
	// and we don't save their orientation anyway. Disable the handle.
	setToolBarsMovable(false);
	
	LyXView::init();
}


void QtView::setWindowTitle(string const & t, string const & it)
{
	setCaption(toqstr(t));
	setIconText(toqstr(it));
}


void QtView::addCommandBuffer(QWidget * parent)
{
	commandbuffer_ = new QCommandBuffer(this, parent, *controlcommand_);
	focus_command_buffer.connect(boost::bind(&QtView::focus_command_widget, this));
}


void QtView::message(string const & str)
{
	statusBar()->message(toqstr(str));
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
	statusBar()->message(toqstr(getLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void QtView::updateStatusBar()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->message(toqstr(getLyXFunc().viewStatusMessage()));
}


void QtView::activated(FuncRequest const & func)
{
	getLyXFunc().dispatch(func);
}


bool QtView::hasFocus() const
{
	return qApp->activeWindow() == this;
}


void QtView::initFloatingGeometry(QRect const & g)
{
	floatingGeometry_ = g;
	maxWidth = QApplication::desktop()->width() - 20;
}


void QtView::updateFloatingGeometry()
{
	if (width() < maxWidth && frameGeometry().x() > 0)
		floatingGeometry_ = QRect(x(), y(), width(), height());
}


void QtView::resizeEvent(QResizeEvent *)
{
	maxWidth = std::max(width(), maxWidth);

	updateFloatingGeometry();
}


void QtView::moveEvent(QMoveEvent *)
{
	updateFloatingGeometry();
}


void QtView::closeEvent(QCloseEvent *)
{
	updateFloatingGeometry();
	QRect geometry = floatingGeometry_;

	Session & session = LyX::ref().session();
	session.saveSessionInfo("WindowIsMaximized", (isMaximized() ? "yes" : "no"));
	// save windows size and position
	session.saveSessionInfo("WindowWidth", convert<string>(geometry.width()));
	session.saveSessionInfo("WindowHeight", convert<string>(geometry.height()));
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
	setCaption(qt_("LyX"));
	QMainWindow::show();
	updateFloatingGeometry();
}


void QtView::busy(bool yes) const
{
	if (yes)
		QApplication::setOverrideCursor(Qt::waitCursor);
	else
		QApplication::restoreOverrideCursor();
}

} // namespace frontend
} // namespace lyx

#include "QtView_moc.cpp"
