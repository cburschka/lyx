/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <sys/time.h>
#include <unistd.h>

#include "LyXView.h"
#include "lyx_main.h"
#if FL_REVISION < 89
#include "lyxlookup.h"
#endif
#include "minibuffer.h"
#include "lyxfunc.h"
#include "debug.h"
#include "layout_forms.h"
#include "intl.h"
#include "lyxrc.h"
#include "support/filetools.h"        // OnlyFilename()
#include "layout.h"
#include "lyxtext.h"
#include "buffer.h"
#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyx_gui_misc.h"	// [update,Close]AllBufferRelatedDialogs
#include "bufferview_funcs.h" // CurrentState()

using std::endl;

extern void AutoSave(BufferView *);
extern void QuitLyX();
LyXTextClass::size_type current_layout = 0;

// This is very temporary
BufferView * current_view;

extern "C" int C_LyXView_atCloseMainFormCB(FL_FORM *, void *);

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
using SigC::slot;
#endif

LyXView::LyXView(int width, int height)
{
	create_form_form_main(width, height);
	fl_set_form_atclose(form_, C_LyXView_atCloseMainFormCB, 0);
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;
	lyxfunc = new LyXFunc(this);
	intl = new Intl;

	// Make sure the buttons are disabled if needed.
	toolbar->update();
	menubar->update();

	dialogs_ = new Dialogs(this);
	// temporary until all dialogs moved into Dialogs.
	dialogs_->updateBufferDependent
		.connect(slot(&updateAllVisibleBufferRelatedDialogs));
	dialogs_->hideBufferDependent
		.connect(slot(&CloseAllBufferRelatedDialogs));
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


/// Redraw the main form.
void LyXView::redraw() {
	lyxerr[Debug::INFO] << "LyXView::redraw()" << endl;
	fl_redraw_form(form_);
	minibuffer->Activate();
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


FL_FORM * LyXView::getForm() const
{
	return form_;
}


Toolbar * LyXView::getToolbar() const
{
	return toolbar;
}


void LyXView::setLayout(LyXTextClass::size_type layout)
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


// Callback for close main form from window manager
int LyXView::atCloseMainFormCB(FL_FORM *, void *)
{
	QuitLyX();
	return FL_IGNORE;
}


// Wrapper for the above
extern "C"
int C_LyXView_atCloseMainFormCB(FL_FORM * form, void * p)
{
	return LyXView::atCloseMainFormCB(form, p);
}


void LyXView::setPosition(int x, int y)
{
	fl_set_form_position(form_, x, y);
}


void LyXView::show(int place, int border, string const & title)
{
	fl_show_form(form_, place, border, title.c_str());
	minibuffer->Init();
#if FL_REVISION < 89
	InitLyXLookup(fl_get_display(), form_->window);
#endif
}


void LyXView::create_form_form_main(int width, int height)
	/* to make this work as it should, .lyxrc should have been
	 * read first; OR maybe this one should be made dynamic.
	 * Hmmmm. Lgb. 
	 * We will probably not have lyxrc before the main form is
	 * initialized, because error messages from lyxrc parsing 
	 * are presented (and rightly so) in GUI popups. Asger. 
	 */
{
	// the main form
	form_ = fl_bgn_form(FL_NO_BOX, width, height);
	form_->u_vdata = this;
	FL_OBJECT * obj = fl_add_box(FL_FLAT_BOX, 0, 0, width, height, "");
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);

	// Parameters for the appearance of the main form
	int const air = 2;
	int const bw = abs(fl_get_border_width());
	
	//
	// THE MENUBAR
	//
	menubar = new Menubar(this, menubackend);

	//
	// TOOLBAR
	//

	toolbar = new Toolbar(this, air, 30 + air + bw, toolbardefaults);

	// Setup the toolbar
	toolbar->set(true);

	//
	// WORKAREA
	//

	int const ywork = 60 + 2 * air + bw;
	int const workheight = height - ywork - (25 + 2 * air);

	::current_view = bufferview = new BufferView(this, air, ywork,
						     width - 3 * air,
						     workheight);

	//
	// MINIBUFFER
	//

	minibuffer = new MiniBuffer(this, air, height - (25 + air), 
				    width - (2 * air), 25);

	//
	// TIMERS
	//

	autosave_timeout.timeout.connect(slot(this, &LyXView::AutoSave));
	
	//
	// Misc
	//

        //  assign an icon to main form
	string iconname = LibFileSearch("images", "lyx", "xpm");
	if (!iconname.empty()) {
		unsigned int w, h;
		Pixmap lyx_p, lyx_mask;
		lyx_p = fl_read_pixmapfile(fl_root,
					   iconname.c_str(),
					   &w,
					   &h,
					   &lyx_mask,
					   0,
					   0,
					   0); // this leaks
		fl_set_form_icon(form_, lyx_p, lyx_mask);
	}

	// set min size
	fl_set_form_minsize(form_, 50, 50);
	
	fl_end_form();
}


void LyXView::init()
{
	// Set the textclass choice
	invalidateLayoutChoice();
	updateLayoutChoice();
	updateMenubar();
	
	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout.setTimeout(lyxrc.autosave * 1000);
		autosave_timeout.start();
	}

        intl->InitKeyMapper(lyxrc.use_kbmap);
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
	} else
		toolbar->updateLayoutList(false);

	

	LyXTextClass::size_type layout =
		bufferview->text->cursor.par()->GetLayout();

	if (layout != current_layout){
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
		string cur_title = buffer()->fileName();
		if (!cur_title.empty()){
			title += ": " + MakeDisplayPath(cur_title, 30);
			if (!buffer()->isLyxClean())
				title += _(" (Changed)");
			if (buffer()->isReadonly())
				title += _(" (read only)");
		}
	}
	// Don't update title if it's the same as last time
	if (title != last_title) {
		fl_set_form_title(form_, title.c_str());
		last_title = title;
	}
}


void LyXView::showState()
{
	getMiniBuffer()->Set(CurrentState(view()));
	getToolbar()->update();
	menubar->update();
}
