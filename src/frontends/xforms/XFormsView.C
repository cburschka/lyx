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
#include "XMiniBuffer.h"

#include "BufferView.h"
#include "debug.h"
#include "lyxfunc.h"
#include "MenuBackend.h"

#include "frontends/Dialogs.h"
#include "frontends/Toolbars.h"

#include "support/filetools.h"        // OnlyFilename()

#include <boost/bind.hpp>

using boost::shared_ptr;

using std::abs;
using std::endl;
using std::string;

//extern void AutoSave(BufferView *);
extern void QuitLyX(bool);

namespace lyx {

using support::LibFileSearch;

namespace frontend {

extern "C" {

static
int C_XFormsView_atCloseMainFormCB(FL_FORM * form, void * p)
{
	return XFormsView::atCloseMainFormCB(form, p);
}

}


void print_metrics(std::ostream & os, std::string const & name, Box const & box)
{
	os << name << " metrics:"
	   << "\tx = " << box.xorigin()
	   << "\ty = " << box.yorigin()
	   << "\tw = " << box.width()
	   << "\th = " << box.height() << '\n';
}


XFormsView::XFormsView(int width, int height)
	: LyXView(),
	  window_(Box(width, height)),
	  icon_pixmap_(0), icon_mask_(0)
{
	int const air = 2;

	// Logical layout of the boxes making up the LyX window.
	shared_ptr<Box> top = window_.children().push_back(Box(0,0));
	shared_ptr<Box> middle = window_.children().push_back(Box(0,0));
	middle->set(Box::Horizontal);
	shared_ptr<Box> bottom = window_.children().push_back(Box(0,0));

	shared_ptr<Box> left = middle->children().push_back(Box(air,0));
	shared_ptr<Box> center = middle->children().push_back(Box(0,0));
	center->set(Box::Expand);
	shared_ptr<Box> right = middle->children().push_back(Box(air,0));

	// Define accessors to the various boxes.
	box_map_[Top]    = top;
	box_map_[Bottom] = bottom;
	box_map_[Left]   = left;
	box_map_[Center] = center;
	box_map_[Right]  = right;

	// Define the XForms components making up the window.
	// Each uses the layout engine defined above to control its
	// dimensions.
	form_ = fl_bgn_form(FL_NO_BOX, width, height);
	form_->u_vdata = this;
	fl_set_form_atclose(form_, C_XFormsView_atCloseMainFormCB, 0);

	FL_OBJECT * obj = fl_add_box(FL_FLAT_BOX, 0, 0, width, height, "");
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);

	menubar_.reset(new XFormsMenubar(this, menubackend));
	getToolbars().init();
	bufferview_.reset(new BufferView(this, width, height));
	minibuffer_.reset(new XMiniBuffer(*this, *controlcommand_));

	//  Assign an icon to the main form.
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
		fl_set_form_icon(form_, icon_pixmap_, icon_mask_);
	}

	fl_end_form();

	// Update the layout so that all widgets fit.
	window_.show();
	updateMetrics(true);

	view_state_con =
		view_state_changed.connect(boost::bind(&XFormsView::show_view_state, this));
	focus_con =
		focus_command_buffer.connect(boost::bind(&XMiniBuffer::focus, minibuffer_.get()));

	// Make sure the buttons are disabled if needed.
	updateToolbars();
	redraw_con =
		getDialogs().redrawGUI().connect(boost::bind(&XFormsView::redraw, this));
}


XFormsView::~XFormsView()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);

	minibuffer_->freeze();
	fl_hide_form(form_);
	fl_free_form(form_);
}


shared_ptr<Box> XFormsView::getBox(Position pos) const
{
	BoxMap::const_iterator it = box_map_.find(pos);
	BOOST_ASSERT(it != box_map_.end());
	return it->second;
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
	QuitLyX(false);
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


void XFormsView::updateMetrics(bool resize_form)
{
	FL_FORM * form = getForm();

	// We don't want the window to be downsized.
	if (!resize_form)
		window_.setMinimumDimensions(form->w, form->h);

	window_.updateMetrics();

	fl_freeze_form(form);

	if (resize_form)
		fl_set_form_size(form, window_.width(), window_.height());

	// Emit a signal so that all daughter widgets are layed-out
	// correctly.
	metricsUpdated();
	fl_unfreeze_form(form);
}


bool XFormsView::hasFocus() const
{
	// No real implementation needed for now
	return true;
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
	message(getLyXFunc().viewStatusMessage());
}


void XFormsView::show_view_state()
{
	message(getLyXFunc().viewStatusMessage());
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

} // namespace frontend
} // namespace lyx
