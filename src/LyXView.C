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
#include "lyxlookup.h"
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
#ifdef NEW_MENUBAR
# include "frontends/Menubar.h"
# include "MenuBackend.h"
#else
# include "menus.h"
#endif
#include "ToolbarDefaults.h"
#include "lyx_gui_misc.h"	// [update,Close]AllBufferRelatedDialogs
#include "bufferview_funcs.h" // CurrentState()

using std::endl;

extern FD_form_document * fd_form_document;

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

	dialogs_ = new Dialogs(this);
	// temporary until all dialogs moved into Dialogs.
	dialogs_->updateBufferDependent
		.connect(slot(&updateAllVisibleBufferRelatedDialogs));
	dialogs_->hideBufferDependent
		.connect(slot(&CloseAllBufferRelatedDialogs));
}


LyXView::~LyXView()
{
#ifdef NEW_MENUBAR
	delete menubar;
#else
	delete menus;
#endif
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

void LyXView::setLayout(int layout)
{
	toolbar->setLayout(layout);
}

LyXFunc * LyXView::getLyXFunc() const
{
	return lyxfunc;
}


MiniBuffer * LyXView::getMiniBuffer() const
{
	return minibuffer;
}


#ifdef NEW_MENUBAR
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

#else
Menus * LyXView::getMenus() const
{
	return menus;
}
#endif


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
extern "C" int C_LyXView_atCloseMainFormCB(FL_FORM * form, void * p)
{
	return LyXView::atCloseMainFormCB(form, p);
}


void LyXView::setPosition(int x, int y)
{
	fl_set_form_position(form_, x, y);
}


void LyXView::show(int place, int border, char const * title)
{
	fl_show_form(form_, place, border, title);
	minibuffer->Init();
	InitLyXLookup(fl_display, form_->window);
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
#ifdef NEW_MENUBAR
	menubar = new Menubar(this, menubackend);
#else	
	menus = new Menus(this, air);
#endif

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


extern "C"
int C_LyXView_KeyPressMask_raw_callback(FL_FORM * fl, void * xev);

void LyXView::init()
{
	// Set the textclass choice
	invalidateLayoutChoice();
	updateLayoutChoice();
	UpdateDocumentClassChoice();
#ifdef NEW_MENUBAR
	updateMenubar();
#endif
	
	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout.setTimeout(lyxrc.autosave * 1000);
		autosave_timeout.start();
	}
	
	// Install the raw callback for keyboard events 
	fl_register_raw_callback(form_,
				 KeyPressMask,
				 C_LyXView_KeyPressMask_raw_callback);
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


void LyXView::UpdateDocumentClassChoice()
{
	// Update the document class display in the document form
	if (fd_form_document) {
		fl_clear_choice(fd_form_document->choice_class);
		for (LyXTextClassList::const_iterator cit = textclasslist.begin();
		     cit != textclasslist.end(); ++cit) {
			fl_addto_choice(fd_form_document->choice_class,
					(*cit).description().c_str());
		}
	}
}


// This is necessary, since FL_FREE-Objects doesn't get all keypress events
// as FL_KEYBOARD events :-(   Matthias 280596
int LyXView::KeyPressMask_raw_callback(FL_FORM * fl, void * xev)
{
	LyXView * view = static_cast<LyXView*>(fl->u_vdata);
	int retval = 0;  // 0 means XForms should have a look at this event

	XKeyEvent * xke = static_cast<XKeyEvent*>(xev);
	static Time last_time_pressed = 0;
	static Time last_time_released = 0;
	static unsigned int last_key_pressed = 0;
	static unsigned int last_key_released = 0;
	static unsigned int last_state_pressed = 0;
	static unsigned int last_state_released = 0;

	// funny. Even though the raw_callback is registered with KeyPressMask,
	// also KeyRelease-events are passed through:-(
	// [It seems that XForms puts them in pairs... (JMarc)]
	if (static_cast<XEvent*>(xev)->type == KeyPress
	    && view->bufferview->focus()
	    && view->bufferview->active())
		{
		last_time_pressed = xke->time;
		last_key_pressed = xke->keycode;
		last_state_pressed = xke->state;
		retval = view->getLyXFunc()
			->processKeyEvent(static_cast<XEvent*>(xev));
	}
	else if (static_cast<XEvent*>(xev)->type == KeyRelease
		 && view->bufferview->focus()
		 && view->bufferview->active())
{
		last_time_released = xke->time;
		last_key_released = xke->keycode;
		last_state_released = xke->state;
	}

	if (last_key_released == last_key_pressed
	    && last_state_released == last_state_pressed
	    && last_time_released == last_time_pressed) {
		// When the diff between last_time_released and
		// last_time_pressed is 0, that sinifies an autoreapeat
		// at least on my system. It like some feedback from
		// others, especially from user running LyX remote.
		//lyxerr << "Syncing - purging X events." << endl;
		XSync(fl_get_display(), 1);
		// This purge make f.ex. scrolling stop imidiatly when
		// releaseing the PageDown button. The question is if this
		// purging of XEvents can cause any harm...after some testing
		// I can see no problems, but I'd like other reports too.
	}
	return retval;
}


// wrapper for the above
extern "C"
int C_LyXView_KeyPressMask_raw_callback(FL_FORM * fl, void * xev)
{
	return LyXView::KeyPressMask_raw_callback(fl, xev);
}


// Updates the title of the window with the filename of the current document
void LyXView::updateWindowTitle()
{
	static string last_title = "LyX";
	string title = "LyX";

	if (view()->available()) {
		string cur_title = buffer()->fileName();
		if (!cur_title.empty()){
			title += ": " + OnlyFilename(cur_title);
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
}
