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

#include "XFormsView.h"
#if FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5)
#include "frontends/xforms/lyxlookup.h"
#endif
#include "minibuffer.h"
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "support/filetools.h"        // OnlyFilename()
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "frontends/Timeout.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyxfunc.h"
#include "BufferView.h"

using std::endl;

//extern void AutoSave(BufferView *);
extern void QuitLyX();

// This is very temporary
BufferView * current_view;

extern "C" {

static
int C_XFormsView_atCloseMainFormCB(FL_FORM * form, void * p)
{
	return XFormsView::atCloseMainFormCB(form, p);
}

}


XFormsView::XFormsView(int width, int height)
	: LyXView()
{
	create_form_form_main(width, height);
	fl_set_form_atclose(form_, C_XFormsView_atCloseMainFormCB, 0);

	// Connect the minibuffer signals
	minibuffer->stringReady.connect(SigC::slot(getLyXFunc(),
						&LyXFunc::miniDispatch));
	minibuffer->timeout.connect(SigC::slot(getLyXFunc(),
					       &LyXFunc::initMiniBuffer));

	// Make sure the buttons are disabled if needed.
	updateToolbar();
}


XFormsView::~XFormsView() {}


/// Redraw the main form.
void XFormsView::redraw() {
	lyxerr[Debug::INFO] << "XFormsView::redraw()" << endl;
	fl_redraw_form(form_);
	getMiniBuffer()->redraw();
}


FL_FORM * XFormsView::getForm() const
{
	return form_;
}


// Callback for close main form from window manager
int XFormsView::atCloseMainFormCB(FL_FORM *, void *)
{
	QuitLyX();
	return FL_IGNORE;
}


void XFormsView::setPosition(int x, int y)
{
	fl_set_form_position(form_, x, y);
}


void XFormsView::show(int place, int border, string const & title)
{
	fl_set_form_minsize(form_, form_->w, form_->h);
	fl_show_form(form_, place, border, title.c_str());
	getLyXFunc()->initMiniBuffer();
#if FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5)
	InitLyXLookup(fl_get_display(), form_->window);
#endif
}


void XFormsView::create_form_form_main(int width, int height)
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

	autosave_timeout->timeout.connect(SigC::slot(this, &XFormsView::AutoSave));

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

	minibuffer->dd_init();
}


void XFormsView::init()
{
	// Set the textclass choice
	invalidateLayoutChoice();
	updateLayoutChoice();
	updateMenubar();

	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout->setTimeout(lyxrc.autosave * 1000);
		autosave_timeout->start();
	}

	intl->InitKeyMapper(lyxrc.use_kbmap);
}


void XFormsView::setWindowTitle(string const & title, string const & icon_title)
{
	fl_set_form_title(form_, title.c_str());
	fl_winicontitle(form_->window, icon_title.c_str());
}


// How should this actually work? Should it prohibit input in all BufferViews,
// or just in the current one? If "just the current one", then it should be
// placed in BufferView. If "all BufferViews" then LyXGUI (I think) should
// run "prohibitInput" on all LyXViews which will run prohibitInput on all
// BufferViews. Or is it perhaps just the (input in) BufferViews in the
// current LyxView that should be prohibited (Lgb) (This applies to
// "allowInput" as well.)
void XFormsView::prohibitInput() const
{
	view()->hideCursor();

	static Cursor cursor;
	static bool cursor_undefined = true;

	if (cursor_undefined) {
		cursor = XCreateFontCursor(fl_get_display(), XC_watch);
		XFlush(fl_get_display());
		cursor_undefined = false;
	}

	/* set the cursor to the watch for all forms and the canvas */
	XDefineCursor(fl_get_display(), getForm()->window, cursor);

	XFlush(fl_get_display());
	fl_deactivate_all_forms();
}


void XFormsView::allowInput() const
{
	/* reset the cursor from the watch for all forms and the canvas */

	XUndefineCursor(fl_get_display(), getForm()->window);

	XFlush(fl_get_display());
	fl_activate_all_forms();
}
