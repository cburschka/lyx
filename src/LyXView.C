// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "lyx_main.h"
#include "lyxlookup.h"
#include "toolbar.h"
#include "minibuffer.h"
#include "lyxfunc.h"
#include "lyx.xpm"
#include "debug.h"
#include "layout_forms.h"
#include "intl.h"
#include "lyxrc.h"
#include "lyxscreen.h"
#include "support/filetools.h"        // OnlyFilename()
#include "layout.h"
#include "lyxtext.h"

extern FD_form_document *fd_form_document;
FD_form_main *fd_form_main; /* a pointer to the one in LyXView
			       should be removed as soon as possible */

MiniBuffer *minibuffer;/* a pointer to the one in LyXView
			  should be removed as soon as possible */

extern void AutoSave();
extern char updatetimer;
extern void QuitLyX();
int current_layout = 0;

// This is very temporary
BufferView *current_view;

extern "C" int C_LyXView_atCloseMainFormCB(FL_FORM *, void *);

LyXView::LyXView(int width, int height)
{
	fd_form_main = create_form_form_main(width,height);
	fl_set_form_atclose(_form, C_LyXView_atCloseMainFormCB, 0);
	lyxerr.debug() << "Initializing LyXFunc" << endl;
	lyxfunc = new LyXFunc(this);
	intl = new Intl;
}


LyXView::~LyXView()
{
	delete menus;
	delete toolbar;
	delete bufferview;
	delete minibuffer;
	delete lyxfunc;
	delete intl;
}

/// Redraw the main form.
void LyXView::redraw() {
	lyxerr.debug() << "LyXView::redraw()" << endl;
	fl_redraw_form(_form);
	minibuffer->Activate();
}


// Callback for update timer
void LyXView::UpdateTimerCB(FL_OBJECT *ob, long)
{
	LyXView *view = (LyXView*) ob->u_vdata;
	if (!view->currentView()->available()) 
		return;
	if (!view->currentView()->getScreen() || !updatetimer)
		return;

	view->currentView()->getScreen()->HideCursor();
	view->currentBuffer()->update(-2);
	/* This update can happen, even when the work area has lost
	 * the focus. So suppress the cursor in that case */
	updatetimer = 0;
}

// Wrapper for the above
extern "C" void C_LyXView_UpdateTimerCB(FL_OBJECT *ob, long data) {
	LyXView::UpdateTimerCB(ob,data);
}



// Callback for autosave timer
void LyXView::AutosaveTimerCB(FL_OBJECT *, long)
{
	lyxerr.debug() << "Running AutoSave()" << endl;
	AutoSave();
}

// Wrapper for the above
extern "C" void C_LyXView_AutosaveTimerCB(FL_OBJECT *ob, long data) {
	LyXView::AutosaveTimerCB(ob, data);
}


/// Reset autosave timer
void LyXView::resetAutosaveTimer()
{
	if (lyxrc->autosave)
		fl_set_timer(_form_main->timer_autosave, lyxrc->autosave);
}


// Callback for close main form from window manager
int LyXView::atCloseMainFormCB(FL_FORM *, void *)
{
	QuitLyX();
	return FL_IGNORE;
}

// Wrapper for the above
extern "C" int C_LyXView_atCloseMainFormCB(FL_FORM *form, void *p) {
	return LyXView::atCloseMainFormCB(form,p);
}


void LyXView::setPosition(int x, int y)
{
	fl_set_form_position(_form, x, y);
}


void LyXView::show(int place, int border, char const* title)
{
	fl_show_form(_form, place, border, title);
	minibuffer->Init();
	InitLyXLookup(fl_display, _form->window);
}


FD_form_main *LyXView::create_form_form_main(int width, int height)
	/* to make this work as it should, .lyxrc should have been
	 * read first; OR maybe this one should be made dynamic.
	 * Hmmmm. Lgb. 
	 * We will probably not have lyxrc before the main form is
	 * initialized, because error messages from lyxrc parsing 
	 * are presented (and rightly so) in GUI popups. Asger. 
	 */
{
	FL_OBJECT *obj;
	
	FD_form_main *fdui = (FD_form_main *)
		fl_calloc(1, sizeof(FD_form_main));

	_form_main = fdui;

	// the main form
	_form = fdui->form_main = fl_bgn_form(FL_NO_BOX, width, height);
	fdui->form_main->u_vdata = (void*) this;
	obj = fl_add_box(FL_FLAT_BOX,0,0,width,height,"");
	fl_set_object_color(obj,FL_MCOL,FL_MCOL);

	// Parameters for the appearance of the main form
	const int air = 2;
	const int bw = abs(fl_get_border_width());
	
	//
	// THE MENUBAR
	//

	menus = new Menus(this, air);

	//
	// TOOLBAR
	//

	toolbar = new Toolbar(lyxrc->toolbar, this, air, 30 + air + bw);

	// Setup the toolbar
	toolbar->set(true);

	//
	// WORKAREA
	//

	const int ywork = 60 + 2*air + bw;
	const int workheight = height - ywork - (25 + 2*air);

	::current_view = bufferview = new BufferView(this, air, ywork,
						     width - 3*air,
						     workheight);

	//
	// MINIBUFFER
	//

	minibuffer = new MiniBuffer(this, air, height-(25+air), 
					  width-(2*air), 25);
	::minibuffer = minibuffer; // to be removed later


	//
	// TIMERS
	//
	
	// timer_autosave
	fdui->timer_autosave = obj = fl_add_timer(FL_HIDDEN_TIMER,
						  0,0,0,0,"Timer");
	fl_set_object_callback(obj,C_LyXView_AutosaveTimerCB,0);
	
	// timer_update
	fdui->timer_update = obj = fl_add_timer(FL_HIDDEN_TIMER,
						0,0,0,0,"Timer");
	fl_set_object_callback(obj,C_LyXView_UpdateTimerCB,0);
	obj->u_vdata = (void*) this;

	//
	// Misc
	//

        //  assign an icon to main form
        unsigned int w, h;
        Pixmap lyx_p, lyx_mask;
        lyx_p = fl_create_from_pixmapdata(fl_root,
					  lyx_xpm,
					  &w,
					  &h,
					  &lyx_mask,
					  0,
					  0,
					  0); // this leaks
        fl_set_form_icon(fdui->form_main, lyx_p, lyx_mask);

	// set min size
	fl_set_form_minsize(fdui->form_main, 50, 50);
	
	fl_end_form();

	return fdui;
}

extern "C" int C_LyXView_KeyPressMask_raw_callback(FL_FORM *fl, void *xev);

void LyXView::init()
{
	// Set the textclass choice
	invalidateLayoutChoice();
	updateLayoutChoice();
	UpdateDocumentClassChoice();
	
	// Start autosave timer
	if (lyxrc->autosave)
		fl_set_timer(_form_main->timer_autosave, lyxrc->autosave);
	
	
	// Install the raw callback for keyboard events 
	fl_register_raw_callback(_form,
				 KeyPressMask,
				 C_LyXView_KeyPressMask_raw_callback);
        intl->InitKeyMapper(lyxrc->use_kbmap);
}


void LyXView::invalidateLayoutChoice()
{
	last_textclass = -1;
}


void LyXView::updateLayoutChoice()
{
	/* update the layout display */
	if (!toolbar->combox) return;

	// this has a bi-effect that the layouts are not showed when no
	// document is loaded.
	if (bufferview==0 || bufferview->currentBuffer()==0) {
		toolbar->combox->clear();
		toolbar->combox->Redraw();
		return;	
	}

	// If textclass is different, we need to update the list
	if (toolbar->combox->empty() ||
	    (last_textclass != int(currentBuffer()->params.textclass))) {
		toolbar->combox->clear();
		for (int i = 0;
		     textclasslist.NameOfLayout(currentBuffer()->
					   params.textclass, i) !="@@end@@";
		     i++) {
			LyXLayout const & layout = textclasslist.
				Style(currentBuffer()->params.textclass, i);
			if (layout.obsoleted_by().empty())
				toolbar->combox->addline(layout.name().c_str());
			else
				toolbar->combox->addline(("@N"+layout.name()).c_str());
		}
		last_textclass = int(currentBuffer()->params.textclass);
		current_layout = 0;
	}
	// we need to do this.
	toolbar->combox->Redraw();
	
	char layout = currentBuffer()->text->cursor.par->GetLayout();

	if (layout != current_layout){
		toolbar->combox->select(layout + 1);
		current_layout = layout;
	}
}


void LyXView::UpdateDocumentClassChoice()
{
	/* update the document class display in the document form */
	int i;
	if (fd_form_document) {
		fl_clear_choice(fd_form_document->choice_class);
		for (i = 0; textclasslist.DescOfClass (i)!="@@end@@"; i++) {
			fl_addto_choice(fd_form_document->choice_class,
					textclasslist.DescOfClass(i).c_str());
		}
	}
}


// This is necessary, since FL_FREE-Objects doesn't get all keypress events
// as FL_KEYBOARD events :-(   Matthias 280596
int LyXView::KeyPressMask_raw_callback(FL_FORM *fl, void *xev)
{
	LyXView *view = (LyXView*) fl->u_vdata;
	int retval = 0;  // 0 means XForms should have a look at this event

	// funny. Even though the raw_callback is registered with KeyPressMask,
	// also KeyRelease-events are passed through:-(
	// [It seems that xforms puts them in pairs... (JMarc)]
	if (((XEvent*)xev)->type == KeyPress
	    && view->bufferview->getWorkArea()->focus
	    && view->bufferview->getWorkArea()->active)
		retval = view->getLyXFunc()->processKeyEvent((XEvent*)xev);

	return retval;
}

// wrapper for the above
extern "C" int C_LyXView_KeyPressMask_raw_callback(FL_FORM *fl, void *xev)
{
	return LyXView::KeyPressMask_raw_callback(fl, xev);
}

// Updates the title of the window with the filename of the current document
void LyXView::updateWindowTitle() {
	static string last_title="LyX";
	string title = "LyX";

	if (currentView()->available()) {
		string cur_title = currentBuffer()->getFileName();
		if (!cur_title.empty()){
			title += ": " + OnlyFilename(cur_title);
			if (!currentBuffer()->isLyxClean())
				title += _(" (Changed)");
			if (currentBuffer()->isReadonly())
				title += _(" (read only)");
		}
	}
	// Don't update title if it's the same as last time
	if (title != last_title) {
		fl_set_form_title(_form, title.c_str());
		last_title = title;
	}
}
