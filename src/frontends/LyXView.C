/**
 * \file LyXView.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjornes
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "minibuffer.h"
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "buffer.h"
#include "MenuBackend.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "lyx_cb.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "lyxtextclasslist.h"

#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Timeout.h"
#include "frontends/Menubar.h"

#include "support/filetools.h" // OnlyFilename()

#include <boost/bind.hpp>

#include <sys/time.h>
#include <unistd.h>

using std::endl;

extern void AutoSave(BufferView *);
extern void QuitLyX();

string current_layout;


LyXView::LyXView()
{
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;

	lyxfunc_.reset(new LyXFunc(this));
	intl_.reset(new Intl);

	// Give the timeout some default sensible value.
	autosave_timeout_.reset(new Timeout(5000));

	dialogs_.reset(new Dialogs(this));
	Dialogs::redrawGUI.connect(boost::bind(&LyXView::redraw, this));
}


LyXView::~LyXView()
{
}


void LyXView::resize()
{
	view()->resize();
}


Buffer * LyXView::buffer() const
{
	return bufferview_->buffer();
}


BufferView * LyXView::view() const
{
	return bufferview_.get();
}


Toolbar * LyXView::getToolbar() const
{
	return toolbar_.get();
}


void LyXView::setLayout(string const & layout)
{
	toolbar_->setLayout(layout);
}


void LyXView::updateToolbar()
{
	toolbar_->update();
}


LyXFunc * LyXView::getLyXFunc() const
{
	return lyxfunc_.get();
}


MiniBuffer * LyXView::getMiniBuffer() const
{
	return minibuffer_.get();
}


void LyXView::message(string const & str)
{
	minibuffer_->message(str);
}


void LyXView::messagePush(string const & str)
{
	minibuffer_->messagePush(str);
}


void LyXView::messagePop()
{
	minibuffer_->messagePop();
}


Menubar * LyXView::getMenubar() const
{
	return menubar_.get();
}


void LyXView::updateMenubar()
{
	if (!view()->buffer() && menubackend.hasMenu("main_nobuffer")) {
		menubar_->set("main_nobuffer");
	} else {
		menubar_->set("main");
	}

	menubar_->update();
}


Intl * LyXView::getIntl() const
{
	return intl_.get();
}


void LyXView::AutoSave()
{
	lyxerr[Debug::INFO] << "Running AutoSave()" << endl;
	if (view()->available())
		::AutoSave(view());
}


void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout_->restart();
}


void LyXView::invalidateLayoutChoice()
{
	last_textclass_ = -1;
}


void LyXView::updateLayoutChoice()
{
	// don't show any layouts without a buffer
	if (!view()->buffer()) {
		toolbar_->clearLayoutList();
		return;
	}

	// update the layout display
	if (last_textclass_ != int(buffer()->params.textclass)) {
		toolbar_->updateLayoutList(true);
		last_textclass_ = int(buffer()->params.textclass);
		current_layout = textclasslist[last_textclass_].defaultLayoutName();
	} else {
		toolbar_->updateLayoutList(false);
	}

	string const & layout =
		bufferview_->getLyXText()->cursor.par()->layout();

	if (layout != current_layout) {
		toolbar_->setLayout(layout);
		current_layout = layout;
	}
}


void LyXView::updateWindowTitle()
{
	static string last_title = "LyX";
	string title = "LyX";
	string icon_title = "LyX";

	if (view()->available()) {
		string const cur_title = buffer()->fileName();
		if (!cur_title.empty()) {
			title += ": " + MakeDisplayPath(cur_title, 30);
			if (!buffer()->isLyxClean())
				title += _(" (changed)");
			if (buffer()->isReadonly())
				title += _(" (read only)");
			// Show only the filename if it's available
			icon_title = OnlyFilename(cur_title);
		}
	}

	if (title != last_title) {
		setWindowTitle(title, icon_title);
		last_title = title;
	}
}


void LyXView::showState()
{
	message(currentState(view()));
}
