/**
 * \file XFormsView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "XFormsView.h"

#include "XFormsMenubar.h"
#include "XFormsToolbar.h"
#include "XMiniBuffer.h"

#include "BufferView.h"
#include "debug.h"
#include "lyxfunc.h"
#include "MenuBackend.h"

#include "frontends/Dialogs.h"

#include "support/filetools.h"        // OnlyFilename()

#include <boost/bind.hpp>

using lyx::support::LibFileSearch;

using std::abs;
using std::endl;
using std::string;


//extern void AutoSave(BufferView *);
extern void QuitLyX();

extern "C" {

static
int C_XFormsView_atCloseMainFormCB(FL_FORM * form, void * p)
{
	return XFormsView::atCloseMainFormCB(form, p);
}

}


XFormsView::XFormsView(int width, int height)
	: LyXView(),
	  icon_pixmap_(0), icon_mask_(0)
{
	create_form_form_main(width, height);
	fl_set_form_atclose(getForm(), C_XFormsView_atCloseMainFormCB, 0);

	view_state_con = view_state_changed.connect(boost::bind(&XFormsView::show_view_state, this));
	focus_con = focus_command_buffer.connect(boost::bind(&XMiniBuffer::focus, minibuffer_.get()));

	// Make sure the buttons are disabled if needed.
	updateToolbar();
	redraw_con = getDialogs().redrawGUI().connect(boost::bind(&XFormsView::redraw, this));
}


XFormsView::~XFormsView()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);

	minibuffer_->freeze();
	fl_hide_form(form_);
	fl_free_form(form_);
}


/// Redraw the main form.
void XFormsView::redraw()
{
	lyxerr[Debug::INFO] << "XFormsView::redraw()" << endl;
	fl_redraw_form(getForm());
	minibuffer_->redraw();
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

	show_view_state();
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
	getForm()->u_vdata = this;
	FL_OBJECT * obj = fl_add_box(FL_FLAT_BOX, 0, 0, width, height, "");
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);

	// Parameters for the appearance of the main form
	int const air = 2;
	int const bw = abs(fl_get_border_width());

	menubar_.reset(new XFormsMenubar(this, menubackend));

	toolbar_.reset(new XFormsToolbar(this, air, 30 + air + bw));
	toolbar_->init();

	int const ywork = 60 + 2 * air + bw;
	int const workheight = height - ywork - (25 + 2 * air);

	bufferview_.reset(new BufferView(this, air, ywork,
		width - 3 * air, workheight));

	minibuffer_.reset(new XMiniBuffer(*controlcommand_,
		air, height - (25 + air), width - (2 * air), 25));

	//  assign an icon to main form
	string const iconname = LibFileSearch("images", "lyx", "xpm");
	if (!iconname.empty()) {
		unsigned int w, h;
		icon_pixmap_ = fl_read_pixmapfile(fl_root,
					   iconname.c_str(),
					   &w,
					   &h,
					   &icon_mask_,
					   0,
					   0,
					   0);
		fl_set_form_icon(getForm(), icon_pixmap_, icon_mask_);
	}

	// set min size
	fl_set_form_minsize(getForm(), 50, 50);

	fl_end_form();
}


void XFormsView::setWindowTitle(string const & title, string const & icon_title)
{
	fl_set_form_title(getForm(), title.c_str());
	fl_winicontitle(form_->window, icon_title.c_str());
}


void XFormsView::message(string const & str)
{
	minibuffer_->message(str);
}


void XFormsView::clearMessage()
{
	message(getLyXFunc().view_status_message());
}


void XFormsView::show_view_state()
{
	message(getLyXFunc().view_status_message());
}


void XFormsView::busy(bool yes) const
{
	if (yes) {
		view()->hideCursor();

		static Cursor cursor;
		static bool cursor_undefined = true;

		if (cursor_undefined) {
			cursor = XCreateFontCursor(fl_get_display(), XC_watch);
			XFlush(fl_get_display());
			cursor_undefined = false;
		}

		/// set the cursor to the watch for all forms and the canvas
		XDefineCursor(fl_get_display(), getForm()->window, cursor);

		XFlush(fl_get_display());

		/// we only need to deactivate to prevent resetting the cursor
		/// to I-beam over the workarea
		fl_deactivate_all_forms();
	} else {
		/// reset the cursor from the watch for all forms and the canvas

		XUndefineCursor(fl_get_display(), getForm()->window);

		XFlush(fl_get_display());
		fl_activate_all_forms();
	}
}
