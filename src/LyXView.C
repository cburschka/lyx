
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

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
#include "bufferview_funcs.h" // CurrentState()
#include "gettext.h"
#include "lyxfunc.h"
#include "BufferView.h"
#include "lyxtextclasslist.h"

#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Timeout.h"
#include "frontends/Menubar.h"

#include "support/filetools.h"        // OnlyFilename()

#include <sys/time.h>
#include <unistd.h>

using std::endl;

extern void AutoSave(BufferView *);
extern void QuitLyX();

string current_layout;


LyXView::LyXView()
{
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;
	lyxfunc = new LyXFunc(this);

	intl = new Intl;

	// Give the timeout some default sensible value.
	autosave_timeout = new Timeout(5000);

	dialogs_ = new Dialogs(this);
	Dialogs::redrawGUI.connect(SigC::slot(this, &LyXView::redraw));
}


LyXView::~LyXView()
{
	delete menubar;
	delete toolbar;
	delete bufferview;
	delete minibuffer;
	delete lyxfunc;
	delete intl;
	delete autosave_timeout;
	delete dialogs_;
}


void LyXView::resize() 
{
	view()->resize();
}


/// returns the buffer currently shown in the main form.
Buffer * LyXView::buffer() const
{
	return bufferview->buffer();
}


BufferView * LyXView::view() const
{
	return bufferview;
}


Toolbar * LyXView::getToolbar() const
{
	return toolbar;
}


void LyXView::setLayout(string const & layout)
{
	toolbar->setLayout(layout);
}


void LyXView::updateToolbar()
{
	toolbar->update();
}


LyXFunc * LyXView::getLyXFunc() const
{
	return lyxfunc;
}


MiniBuffer * LyXView::getMiniBuffer() const
{
	return minibuffer;
}


void LyXView::message(string const & str)
{
	minibuffer->message(str);
}


void LyXView::messagePush(string const & str)
{
	minibuffer->messagePush(str);
}


void LyXView::messagePop()
{
	minibuffer->messagePop();
}


Menubar * LyXView::getMenubar() const
{
	return menubar;
}


void LyXView::updateMenubar() 
{
	if ((!view() || !view()->buffer())
	    && menubackend.hasMenu("main_nobuffer"))
		menubar->set("main_nobuffer");
	else
		menubar->set("main");
	menubar->update();
}


Intl * LyXView::getIntl() const
{
	return intl;
}


// Callback for autosave timer
void LyXView::AutoSave()
{
	lyxerr[Debug::INFO] << "Running AutoSave()" << endl;
	if (view()->available())
		::AutoSave(view());
}


/// Reset autosave timer
void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout->restart();
}


void LyXView::invalidateLayoutChoice()
{
	last_textclass = -1;
}


void LyXView::updateLayoutChoice()
{
	// This has a side-effect that the layouts are not showed when no
	// document is loaded.
	if (!view() || !view()->buffer()) {
		toolbar->clearLayoutList();
		return;	
	}

	// Update the layout display
	if (last_textclass != int(buffer()->params.textclass)) {
		toolbar->updateLayoutList(true);
		last_textclass = int(buffer()->params.textclass);
		current_layout = textclasslist[last_textclass].defaultLayoutName();
	} else {
		toolbar->updateLayoutList(false);
	}

	string const & layout =
		bufferview->getLyXText()->cursor.par()->layout();

	if (layout != current_layout) {
		toolbar->setLayout(layout);
		current_layout = layout;
	}
}


// Updates the title of the window with the filename of the current document
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
				title += _(" (Changed)");
			if (buffer()->isReadonly())
				title += _(" (read only)");
			/* Show only the filename if it's available. */
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
