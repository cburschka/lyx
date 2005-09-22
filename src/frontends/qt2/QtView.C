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
#include "lyxfunc.h"
#include "MenuBackend.h"

#include "frontends/Toolbars.h"

#include "support/filetools.h"

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

using support::LibFileSearch;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon



QtView::QtView(unsigned int width, unsigned int height)
	: QMainWindow(), LyXView(), commandbuffer_(0)
{
	resize(width, height);

	qApp->setMainWidget(this);

	bufferview_.reset(new BufferView(this, width, height));

	menubar_.reset(new QLMenubar(this, menubackend));
	getToolbars().init();

	statusBar()->setSizeGripEnabled(false);

	view_state_changed.connect(boost::bind(&QtView::update_view_state, this));
	connect(&statusbar_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));

#ifndef Q_WS_MACX
	//  assign an icon to main form. We do not do it under Qt/Mac,
	//  since the icon is provided in the application bundle.
	string const iconname = LibFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setIcon(QPixmap(toqstr(iconname)));
#endif

	// make sure the buttons are disabled if needed
	updateToolbars();

	// allowing the toolbars to tear off is too easily done,
	// and we don't save their orientation anyway. Disable the handle.
	setToolBarsMovable(false);
}


QtView::~QtView()
{
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


void QtView::update_view_state()
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


void QtView::closeEvent(QCloseEvent *)
{
	QuitLyX(false);
}


void QtView::show()
{
	setCaption(qt_("LyX"));
	QMainWindow::show();
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
