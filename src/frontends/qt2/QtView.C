/**
 * \file QtView.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjornes <larsbj@lyx.org>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
#include "intl.h"
#include "lyx_cb.h"
#include "support/filetools.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyxfunc.h"
#include "bufferview_funcs.h"
#include "BufferView.h"

#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "frontends/Dialogs.h"
#include "frontends/Timeout.h"
 
#include <boost/bind.hpp>
 
#include "QtView.h"
#include "qfont_loader.h"
#include "QCommandBuffer.h"
 
#include <qapplication.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qstatusbar.h>
 
using std::endl;

// FIXME: this has to go away
BufferView * current_view;

qfont_loader fontloader;

QtView::QtView(unsigned int width, unsigned int height)
	: QMainWindow(), LyXView()
{
	resize(width, height);
 
	qApp->setMainWidget(this);
 
	menubar_.reset(new Menubar(this, menubackend));

	connect(menuBar(), SIGNAL(activated(int)),
		this, SLOT(activated(int)));
 
	toolbar_.reset(new Toolbar(this, *getDialogs(), 0, 0, toolbardefaults));
	toolbar_->set(true);

	statusBar()->setSizeGripEnabled(false);
 
	bufferview_.reset(new BufferView(this, 0, 0, width, height));
	::current_view = bufferview_.get();

	view_state_changed.connect(boost::bind(&QtView::update_view_state, this));
	connect(&idle_timer_, SIGNAL(timeout()), this, SLOT(update_view_state_qt()));
 
	idle_timer_.start(3000); 
 
	focus_command_buffer.connect(boost::bind(&QtView::focus_command_widget, this));
 
	commandbuffer_ = new QCommandBuffer(this, *controlcommand_);
 
	addToolBar(commandbuffer_, Bottom, true);
 
        //  assign an icon to main form
	string const iconname = LibFileSearch("images", "lyx", "xpm");
 
	if (!iconname.empty())
		setIcon(QPixmap(iconname.c_str()));

	// make sure the buttons are disabled if needed
	updateToolbar();
}


QtView::~QtView()
{
}


void QtView::message(string const & str)
{
	statusBar()->message(str.c_str()); 
	idle_timer_.stop();
	idle_timer_.start(3000);
}

 
void QtView::focus_command_widget()
{
	commandbuffer_->focus_command();
}


void QtView::update_view_state_qt()
{
	statusBar()->message(currentState(view()).c_str());
}

 
void QtView::update_view_state()
{
	statusBar()->message(currentState(view()).c_str());
}

 
void QtView::activated(int id)
{
	getLyXFunc()->dispatch(id, true);
}
 
 
void QtView::closeEvent(QCloseEvent *)
{
	QuitLyX();
}

 
void QtView::show(int x, int y, string const & title)
{
	move(x, y);
	setCaption(title.c_str());
	QMainWindow::show();
}


void QtView::prohibitInput() const
{
	// FIXME
	//setFocusPolicy(QWidget::NoFocus);
}


void QtView::allowInput() const
{
	// FIXME
	//setFocusPolicy(QWidget::strongFocus);
}
