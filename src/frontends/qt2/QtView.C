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

//#include "minibuffer.h"
#include "debug.h"
#include "intl.h"
#include "lyx_cb.h"
#include "support/filetools.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyxfunc.h"
#include "BufferView.h"

#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "frontends/Dialogs.h"
#include "frontends/Timeout.h"
 
#include "QtView.h"
#include "qfont_loader.h"
 
#include <qapplication.h>
#include <qpixmap.h>
#include <qmenubar.h>
 
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
 
	toolbar_.reset(new Toolbar(this, 0, 0, toolbardefaults));
	toolbar_->set(true);

	// FIXME minibuffer_.reset(new MiniBuffer(this, 0, 0, 30, 30));
 
	bufferview_.reset(new BufferView(this, 0, 0, width, height));
	::current_view = bufferview_.get();

	// FIXME: move 
	// FIXME autosave_timeout_->timeout.connect(SigC::slot(this, &QtView::autoSave));
	
        //  assign an icon to main form
	string const iconname = LibFileSearch("images", "lyx", "xpm");
 
	if (!iconname.empty())
		setIcon(QPixmap(iconname.c_str()));

#if 0
	minibuffer_.reset(new MiniBuffer(this, air, height - (25 + air), 
		width - (2 * air), 25));

	// connect the minibuffer signals
	minibuffer_->stringReady.connect(SigC::slot(getLyXFunc(),
						&LyXFunc::miniDispatch));
	minibuffer_->timeout.connect(SigC::slot(getLyXFunc(),
					       &LyXFunc::initMiniBuffer));
	minibuffer_->dd_init(); 
 
#endif
	
	// make sure the buttons are disabled if needed
	updateToolbar();
}


QtView::~QtView()
{
}


void QtView::activated(int id)
{
	getLyXFunc()->verboseDispatch(id, true);
}
 
 
void QtView::closeEvent(QCloseEvent *)
{
	QuitLyX();
}

 
void QtView::redraw() 
{
	// FIXME: does this make any sense for us ? 
	repaint();
}


void QtView::setPosition(unsigned int x, unsigned int y)
{
	move(x, y);
}


void QtView::show(int, int, string const & title)
{
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
