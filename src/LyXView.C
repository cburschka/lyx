
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

#include <sys/time.h>
#include <unistd.h>

#include "LyXView.h"
#include "minibuffer.h"
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "support/filetools.h"        // OnlyFilename()
#include "lyxtext.h"
#include "buffer.h"
#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "MenuBackend.h"
#include "lyx_gui_misc.h"	// [update,Close,Redraw]AllBufferRelatedDialogs
#include "bufferview_funcs.h" // CurrentState()
#include "gettext.h"
#include "lyxfunc.h"
#include "BufferView.h"

using std::endl;
using lyx::layout_type;

extern void AutoSave(BufferView *);
extern void QuitLyX();

layout_type current_layout = 0;


LyXView::LyXView()
{
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;
	lyxfunc = new LyXFunc(this);

	intl = new Intl;

	dialogs_ = new Dialogs(this);
	// temporary until all dialogs moved into Dialogs.
	dialogs_->updateBufferDependent
		.connect(SigC::slot(&updateAllVisibleBufferRelatedDialogs));
	dialogs_->hideBufferDependent
		.connect(SigC::slot(&CloseAllBufferRelatedDialogs));
	Dialogs::redrawGUI.connect(SigC::slot(this, &LyXView::redraw));
	Dialogs::redrawGUI.connect(SigC::slot(&RedrawAllBufferRelatedDialogs));
}


LyXView::~LyXView()
{
	delete menubar;
	delete toolbar;
	delete bufferview;
	delete minibuffer;
	delete lyxfunc;
	delete intl;
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


void LyXView::setLayout(layout_type layout)
{
	toolbar->setLayout(layout);
}


void LyXView::updateToolbar()
{
	toolbar->update();
	menubar->update();
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
		autosave_timeout.restart();
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
		current_layout = 0;
	} else {
		toolbar->updateLayoutList(false);
	}
	
	layout_type layout = bufferview->getLyXText()->cursor.par()->getLayout();

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

	if (view()->available()) {
		string const cur_title = buffer()->fileName();
		if (!cur_title.empty()){
			title += ": " + MakeDisplayPath(cur_title, 30);
			if (!buffer()->isLyxClean())
				title += _(" (Changed)");
			if (buffer()->isReadonly())
				title += _(" (read only)");
		}
	}
	if (title != last_title) {
		setWindowTitle(title);
		last_title = title;
	}
	last_title = title;
}


void LyXView::showState()
{
	message(currentState(view()));
	getToolbar()->update();
	menubar->update();
}
