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

#include "lyx_cb.h"
#include "support/filetools.h"
#include "MenuBackend.h"
#include "lyxfunc.h"
#include "BufferView.h"

#include <boost/bind.hpp>

#include "QtView.h"
#include "QLToolbar.h"
#include "QLMenubar.h"
#include "qfont_loader.h"
#include "QCommandBuffer.h"

#include <qapplication.h>
#include <qstatusbar.h>

using lyx::support::LibFileSearch;


namespace {

int const statusbar_timer_value = 3000;

} // namespace anon

qfont_loader fontloader;


QtView::QtView(unsigned int width, unsigned int height)
	: QMainWindow(), LyXView(), commandbuffer_(0)
{
	resize(width, height);

	qApp->setMainWidget(this);

	bufferview_.reset(new BufferView(this, 0, 0, width, height));

	menubar_.reset(new QLMenubar(this, menubackend));
	toolbar_.reset(new QLToolbar(this));
	toolbar_->init();

	statusBar()->setSizeGripEnabled(false);

	view_state_changed.connect(boost::bind(&QtView::update_view_state, this));
	connect(&statusbar_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));

	//  assign an icon to main form
	string const iconname = LibFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setIcon(QPixmap(toqstr(iconname)));

	// make sure the buttons are disabled if needed
	updateToolbar();

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
	statusBar()->message(toqstr(getLyXFunc().view_status_message()));
	statusbar_timer_.stop();
}


void QtView::update_view_state()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->message(toqstr(getLyXFunc().view_status_message()));
}


void QtView::activated(FuncRequest const & func)
{
	getLyXFunc().dispatch(func, true);
}


void QtView::closeEvent(QCloseEvent *)
{
	QuitLyX();
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
