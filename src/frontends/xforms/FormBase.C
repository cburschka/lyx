/**
 * \file FormBase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormBase.h"

#include "Tooltips.h"
#include "xforms_helpers.h" // formatted
#include "xforms_resize.h"
#include "xformsBC.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlButtons.h"

#include "support/filetools.h" //  LibFileSearch
#include "support/lstrings.h"

#include "lyx_forms.h"

using lyx::support::bformat;
using lyx::support::LibFileSearch;


extern "C" {

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL == 0)
// These should be in forms.h but aren't
void fl_show_tooltip(const char *, int, int);
void fl_hide_tooltip();
#endif

// Callback function invoked by xforms when the dialog is closed by the
// window manager.
static int C_WMHideCB(FL_FORM * form, void *);

// Callback function invoked by the xforms pre-handler routine.
static int C_PrehandlerCB(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

} // extern "C"


FormBase::FormBase(string const & t, bool allowResize)
	: ViewBase(t),
	  warning_posted_(false), message_widget_(0),
	  minw_(0), minh_(0), allow_resize_(allowResize),
	  icon_pixmap_(0), icon_mask_(0),
	  tooltips_(new Tooltips())
{}


FormBase::~FormBase()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);

	delete tooltips_;
}


bool FormBase::isVisible() const
{
	return form() && form()->visible;
}


Tooltips & FormBase::tooltips()
{
	return *tooltips_;
}


void FormBase::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


xformsBC & FormBase::bcview()
{
	return static_cast<xformsBC &>(bc().view());
}


void FormBase::prepare_to_show()
{
	double const scale = get_scale_to_fit(form());
	if (scale > 1.001)
		scale_form_horizontally(form(), scale);

	// work around dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_form_atclose(form(), C_WMHideCB, 0);

	// set the title for the minimized form
	if (!getController().IconifyWithMain())
		fl_winicontitle(form()->window, getTitle().c_str());

	//  assign an icon to the form
	string const iconname = LibFileSearch("images", "lyx", "xpm");
	if (!iconname.empty()) {
		unsigned int w, h;
		icon_pixmap_ = fl_read_pixmapfile(fl_root,
						  iconname.c_str(),
						  &w,
						  &h,
						  &icon_mask_,
						  0, 0, 0);
		fl_set_form_icon(form(), icon_pixmap_, icon_mask_);
	}
}


void FormBase::show()
{
	// build() is/should be called from the controller, so form() should
	// always exist.
	BOOST_ASSERT(form());

	// we use minw_ to flag whether the dialog has ever been shown.
	// In turn, prepare_to_show() initialises various bits 'n' pieces
	// (including minw_).
	if (minw_ == 0) {
		prepare_to_show();
	}

	// make sure the form is up to date.
	fl_freeze_form(form());
	update();
	fl_unfreeze_form(form());

	if (form()->visible) {
		fl_raise_form(form());
		/* This XMapWindow() will hopefully ensure that
		 * iconified dialogs are de-iconified. Mad props
		 * out to those crazy Xlib guys for forgetting a
		 * XDeiconifyWindow(). At least WindowMaker, when
		 * being notified of the redirected MapRequest will
		 * specifically de-iconify. From source, fvwm2 seems
		 * to do the same.
		 */
		XMapWindow(fl_get_display(), form()->window);
	} else {
		// calls to fl_set_form_minsize/maxsize apply only to the next
		// fl_show_form(), so this comes first.
		fl_set_form_minsize(form(), minw_, minh_);
		if (!allow_resize_)
			fl_set_form_maxsize(form(), minw_, minh_);

		string const maximize_title = "LyX: " + getTitle();
		int const iconify_policy =
			getController().IconifyWithMain() ? FL_TRANSIENT : 0;

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     iconify_policy,
			     maximize_title.c_str());
	}
}


void FormBase::hide()
{
	// xforms sometimes tries to process a hint-type MotionNotify, and
	// use XQueryPointer, without verifying if the window still exists.
	// So we try to clear out motion events in the queue before the
	// DestroyNotify
	XSync(fl_get_display(), false);

	if (form() && form()->visible)
		fl_hide_form(form());
}


void FormBase::setPrehandler(FL_OBJECT * ob)
{
	BOOST_ASSERT(ob);
	fl_set_object_prehandler(ob, C_PrehandlerCB);
}


void FormBase::setMessageWidget(FL_OBJECT * ob)
{
	BOOST_ASSERT(ob && ob->objclass == FL_TEXT);
	message_widget_ = ob;
	fl_set_object_lsize(message_widget_, FL_NORMAL_SIZE);
}


void FormBase::InputCB(FL_OBJECT * ob, long data)
{
	// It is possible to set the choice to 0 when using the
	// keyboard shortcuts. This work-around deals with the problem.
	if (ob && ob->objclass == FL_CHOICE && fl_get_choice(ob) < 1) {
		fl_set_choice(ob, 1);
	}

	bc().input(input(ob, data));
}


ButtonPolicy::SMInput FormBase::input(FL_OBJECT *, long)
{
	return ButtonPolicy::SMI_VALID;
}


// preemptive handler for feedback messages
void FormBase::MessageCB(FL_OBJECT * ob, int event)
{
	BOOST_ASSERT(ob);

	switch (event) {
	case FL_ENTER:
	{
		string const feedback = getFeedback(ob);
		if (feedback.empty() && warning_posted_)
			break;

		warning_posted_ = false;
		postMessage(getFeedback(ob));
		break;
	}

	case FL_LEAVE:
		if (!warning_posted_)
			clearMessage();
		break;

	default:
		break;
	}
}


void FormBase::PrehandlerCB(FL_OBJECT * ob, int event, int key)
{
	BOOST_ASSERT(ob);

	if (ob->objclass == FL_INPUT && event == FL_PUSH && key == 2) {
		// Trigger an input event when pasting in an xforms input object
		// using the middle mouse button.
		InputCB(ob, 0);
		return;
	}

	if (message_widget_) {
		switch (event) {
		case FL_ENTER:
		case FL_LEAVE:
			// Post feedback as the mouse enters the object,
			// remove it as the mouse leaves.
			MessageCB(ob, event);
			break;
		}
	}

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL == 0)
	// Tooltips are not displayed on browser widgets due to an xforms' bug.
	// This is a work-around:
	if (ob->objclass == FL_BROWSER) {
		switch (event) {
		case FL_ENTER:
			if (ob->tooltip && *(ob->tooltip)) {
				int const x = ob->form->x + ob->x;
				int const y = ob->form->y + ob->y + ob->h + 1;
				fl_show_tooltip(ob->tooltip, x, y);
			}
			break;
		case FL_LEAVE:
		case FL_PUSH:
		case FL_KEYPRESS:
			fl_hide_tooltip();
			break;
		}
	}
#endif
}


void FormBase::postWarning(string const & warning)
{
	warning_posted_ = true;
	postMessage(warning);
}


void FormBase::clearMessage()
{
	BOOST_ASSERT(message_widget_);

	warning_posted_ = false;

	string const existing = message_widget_->label
		? message_widget_->label : string();
	if (existing.empty())
		return;

	// This trick is needed to get xforms to clear the label...
	fl_set_object_label(message_widget_, "");
	fl_hide_object(message_widget_);
}


void FormBase::postMessage(string const & message)
{
	BOOST_ASSERT(message_widget_);

	int const width = message_widget_->w - 10;
	string const tmp = warning_posted_ ?
		bformat(_("WARNING! %1$s"), message) :
		message;

	string const str = formatted(tmp, width, FL_NORMAL_SIZE);

	fl_set_object_label(message_widget_, str.c_str());
	FL_COLOR const label_color = warning_posted_ ? FL_RED : FL_LCOL;
	fl_set_object_lcol(message_widget_, label_color);

	if (!message_widget_->visible)
		fl_show_object(message_widget_);
}


namespace {

FormBase * GetForm(FL_OBJECT * ob)
{
	BOOST_ASSERT(ob && ob->form && ob->form->u_vdata);
	FormBase * ptr = static_cast<FormBase *>(ob->form->u_vdata);
	return ptr;
}

} // namespace anon


extern "C" {

void C_FormBaseApplyCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->getController().ApplyButton();
}


void C_FormBaseOKCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->getController().OKButton();
}


void C_FormBaseCancelCB(FL_OBJECT * ob, long)
{
	FormBase * form = GetForm(ob);
	form->getController().CancelButton();
}


void C_FormBaseRestoreCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->getController().RestoreButton();
}


void C_FormBaseInputCB(FL_OBJECT * ob, long d)
{
	GetForm(ob)->InputCB(ob, d);
}


static int C_WMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	BOOST_ASSERT(form && form->u_vdata);
	FormBase * ptr = static_cast<FormBase *>(form->u_vdata);
	ptr->getController().CancelButton();
	return FL_CANCEL;
}

static int C_PrehandlerCB(FL_OBJECT * ob, int event,
			  FL_Coord, FL_Coord, int key, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.
	BOOST_ASSERT(ob);

	// Don't Assert this one, as it can happen quite naturally when things
	// are being deleted in the d-tor.
	//BOOST_ASSERT(ob->form);
	if (!ob->form) return 0;

	FormBase * ptr = static_cast<FormBase *>(ob->form->u_vdata);

	if (ptr)
		ptr->PrehandlerCB(ob, event, key);

	return 0;
}

} // extern "C"
