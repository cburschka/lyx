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
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "buffer.h"
#include "MenuBackend.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "funcrequest.h"
#include "lyx_cb.h"
#include "BufferView.h"
#include "bufferview_funcs.h"

#include "Dialogs.h"
#include "Toolbar.h"
#include "Timeout.h"
#include "Menubar.h"
#include "controllers/ControlCommandBuffer.h"

#include "support/filetools.h" // OnlyFilename()

#include <boost/bind.hpp>

#include <sys/time.h>
#include <unistd.h>

using std::endl;

string current_layout;


LyXView::LyXView()
	: intl_(new Intl),
	  autosave_timeout_(new Timeout(5000)),
	  lyxfunc_(new LyXFunc(this)),
	  dialogs_(new Dialogs(*this)),
	  controlcommand_(new ControlCommandBuffer(getLyXFunc()))
{
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;
}


LyXView::~LyXView()
{
}


void LyXView::init()
{
	updateLayoutChoice();
	updateMenubar();

	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout_->timeout.connect(boost::bind(&LyXView::autoSave, this));
		autosave_timeout_->setTimeout(lyxrc.autosave * 1000);
		autosave_timeout_->start();
	}

	intl_->InitKeyMapper(lyxrc.use_kbmap);
}


Buffer * LyXView::buffer() const
{
	return bufferview_->buffer();
}


boost::shared_ptr<BufferView> const & LyXView::view() const
{
	return bufferview_;
}


void LyXView::setLayout(string const & layout)
{
	toolbar_->setLayout(layout);
}


void LyXView::updateToolbar()
{
	toolbar_->update();
}


void LyXView::updateMenubar()
{
	menubar_->update();
}


void LyXView::autoSave()
{
	lyxerr[Debug::INFO] << "Running autoSave()" << endl;

	if (view()->available()) {
		::AutoSave(view().get());
	}
}


void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout_->restart();
}


void LyXView::updateLayoutChoice()
{
	// don't show any layouts without a buffer
	if (!view()->buffer()) {
		toolbar_->clearLayoutList();
		return;
	}

	// update the layout display
	if (toolbar_->updateLayoutList(buffer()->params.textclass)) {
		current_layout = buffer()->params.getLyXTextClass().defaultLayoutName();
	}

	string const & layout =
		bufferview_->getLyXText()->cursor.par()->layout()->name();

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
			if (!buffer()->isClean())
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


void LyXView::dispatch(FuncRequest const & req)
{
	// substitute the correct BufferView here
	FuncRequest r = req;
	r.setView(view().get());
	getLyXFunc().dispatch(r); 
}
