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
#include FORMS_H_LOCATION
#if FL_VERSION < 1 && (FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5))
#include "frontends/xforms/lyxlookup.h"
#endif
#include "frontends/MiniBuffer.h"
#include "frontends/xforms/XMiniBuffer.h"
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "support/filetools.h"        // OnlyFilename()
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "frontends/Timeout.h"
#include "frontends/Dialogs.h"
#include "MenuBackend.h"
#include "ToolbarDefaults.h"
#include "lyxfunc.h"
#include "BufferView.h"

#include <boost/bind.hpp>

using std::abs;
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
	create_form_form_main(*getDialogs(), width, height);
	fl_set_form_atclose(getForm(), C_XFormsView_atCloseMainFormCB, 0);

	// Connect the minibuffer signals
	minibuffer_->inputReady.connect(boost::bind(&LyXFunc::miniDispatch, getLyXFunc(), _1));
	minibuffer_->timeout.connect(boost::bind(&LyXFunc::initMiniBuffer, getLyXFunc()));

	// Make sure the buttons are disabled if needed.
	updateToolbar();
	getDialogs()->redrawGUI.connect(boost::bind(&XFormsView::redraw, this));
}


XFormsView::~XFormsView()
{
	fl_hide_form(form_);
	fl_free_form(form_);
}


/// Redraw the main form.
void XFormsView::redraw()
{
	lyxerr[Debug::INFO] << "XFormsView::redraw()" << endl;
	fl_redraw_form(getForm());
	// This is dangerous, but we know it is safe
	XMiniBuffer * m = static_cast<XMiniBuffer *>(getMiniBuffer());
	m->redraw();
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


void XFormsView::show(int x, int y, string const & title)
{
	FL_FORM * form = getForm();

	fl_set_form_minsize(form, form->w, form->h);

	int placement = FL_PLACE_CENTER | FL_FREE_SIZE;

	// Did we get a valid geometry position ?
	if (x >= 0 && y >= 0) {
		fl_set_form_position(form, x, y);
		placement = FL_PLACE_POSITION;
	}

	fl_show_form(form, placement, FL_FULLBORDER, title.c_str());

	getLyXFunc()->initMiniBuffer();
#if FL_VERSION < 1 && (FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5))
	InitLyXLookup(fl_get_display(), form_->window);
#endif
}


void XFormsView::create_form_form_main(Dialogs & dia, int width, int height)
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
	getForm()->u_vdata = this;
	FL_OBJECT * obj = fl_add_box(FL_FLAT_BOX, 0, 0, width, height, "");
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);

	// Parameters for the appearance of the main form
	int const air = 2;
	int const bw = abs(fl_get_border_width());

	menubar_.reset(new Menubar(this, menubackend));

	toolbar_.reset(new Toolbar(this, dia,
				   air, 30 + air + bw, toolbardefaults));
	toolbar_->set(true);

	int const ywork = 60 + 2 * air + bw;
	int const workheight = height - ywork - (25 + 2 * air);

	bufferview_.reset(new BufferView(this, air, ywork,
		width - 3 * air, workheight));
	::current_view = bufferview_.get();

	minibuffer_.reset(new XMiniBuffer(this, air, height - (25 + air),
		width - (2 * air), 25));

	// FIXME: why do this in xforms/ ?
	autosave_timeout_->timeout.connect(boost::bind(&XFormsView::autoSave, this));

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
		fl_set_form_icon(getForm(), lyx_p, lyx_mask);
	}

	// set min size
	fl_set_form_minsize(getForm(), 50, 50);

	fl_end_form();

	// This is dangerous, but we know it is safe in this situation
	static_cast<XMiniBuffer *>(minibuffer_.get())->dd_init();
}


void XFormsView::setWindowTitle(string const & title, string const & icon_title)
{
	fl_set_form_title(getForm(), title.c_str());
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
