/**
 * \file FormDialogView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "FormDialogView.h"

#include "Dialog.h"
#include "xformsBC.h"
#include "ButtonController.h"
#include "xforms_resize.h"
#include "Tooltips.h"
#include "xforms_helpers.h" // formatted

#include "gettext.h"        // _()
#include "lyxrc.h"
#include "BoostFormat.h"

#include "support/LAssert.h"
#include "support/filetools.h" //  LibFileSearch

#include FORMS_H_LOCATION

extern "C" {

// These should be in forms.h but aren't
void fl_show_tooltip(const char *, int, int);

void fl_hide_tooltip();

// Callback function invoked by xforms when the dialog is closed by the
// window manager.
static int C_WMHideCB(FL_FORM * form, void *);

// Callback function invoked by the xforms pre-handler routine.
static int C_PrehandlerCB(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

} // extern "C"


FormDialogView::FormDialogView(Dialog & parent,
			       string const & t, bool allowResize)
	: Dialog::View(parent),
	  warning_posted_(false), message_widget_(0),
	  minw_(0), minh_(0), allow_resize_(allowResize),
	  title_(t), icon_pixmap_(0), icon_mask_(0),
	  tooltips_(new Tooltips())
{}


FormDialogView::~FormDialogView()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);

	delete tooltips_;
}


bool FormDialogView::isVisible() const
{
	return form() && form()->visible;
}


Tooltips & FormDialogView::tooltips()
{
	return *tooltips_;
}


void FormDialogView::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


xformsBC & FormDialogView::bcview()
{
	return static_cast<xformsBC &>(dialog().bc().view());
}


void FormDialogView::prepare_to_show()
{
	double const scale = get_scale_to_fit(form());
	if (scale > 1.001)
		scale_form_horizontally(form(), scale);

	// work around dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_form_atclose(form(), C_WMHideCB, 0);

	// set the title for the minimized form
	if (!lyxrc.dialogs_iconify_with_main)
		fl_winicontitle(form()->window, title_.c_str());

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


void FormDialogView::show()
{
	if (!form()) {
		build();
	}

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

		string const maximize_title = "LyX: " + title_;
		int const iconify_policy = lyxrc.dialogs_iconify_with_main ?
			FL_TRANSIENT : 0;

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     iconify_policy,
			     maximize_title.c_str());
	}
}


void FormDialogView::hide()
{
	// xforms sometimes tries to process a hint-type MotionNotify, and
	// use XQueryPointer, without verifying if the window still exists.
	// So we try to clear out motion events in the queue before the
	// DestroyNotify
	XSync(fl_get_display(), false);

	if (form() && form()->visible)
		fl_hide_form(form());
}


void FormDialogView::setPrehandler(FL_OBJECT * ob)
{
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_PrehandlerCB);
}


void FormDialogView::setMessageWidget(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->objclass == FL_TEXT);
	message_widget_ = ob;
	fl_set_object_lsize(message_widget_, FL_NORMAL_SIZE);
}


void FormDialogView::InputCB(FL_OBJECT * ob, long data)
{
	// It is possible to set the choice to 0 when using the
	// keyboard shortcuts. This work-around deals with the problem.
	if (ob && ob->objclass == FL_CHOICE && fl_get_choice(ob) < 1) {
		fl_set_choice(ob, 1);
	}

	bc().input(input(ob, data));
}


ButtonPolicy::SMInput FormDialogView::input(FL_OBJECT *, long)
{
	return ButtonPolicy::SMI_VALID;
}


// preemptive handler for feedback messages
void FormDialogView::MessageCB(FL_OBJECT * ob, int event)
{
	lyx::Assert(ob);

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


void FormDialogView::PrehandlerCB(FL_OBJECT * ob, int event, int key)
{
	lyx::Assert(ob);

	if (ob->objclass == FL_INPUT && event == FL_PUSH && key == 2) {
		// Trigger an input event when pasting in an xforms input
		// object using the middle mouse button.
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

	// Tooltips are not displayed on browser widgets due to an xforms' bug.
	// I have a fix, but it's not yet in the xforms sources.
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
}


void FormDialogView::postWarning(string const & warning)
{
	warning_posted_ = true;
	postMessage(warning);
}


void FormDialogView::clearMessage()
{
	lyx::Assert(message_widget_);

	warning_posted_ = false;

	string const existing = message_widget_->label
		? message_widget_->label : string();
	if (existing.empty())
		return;

	// This trick is needed to get xforms to clear the label...
	fl_set_object_label(message_widget_, "");
	fl_hide_object(message_widget_);
}


void FormDialogView::postMessage(string const & message)
{
	lyx::Assert(message_widget_);

	int const width = message_widget_->w - 10;
#if USE_BOOST_FORMAT
	boost::format fmter = warning_posted_ ?
		boost::format(_("WARNING! %1$s")) :
		boost::format("%1$s");

	string const str = formatted(boost::io::str(fmter % message),
				     width, FL_NORMAL_SIZE);
#else
	string const tmp = warning_posted_ ?
		_("WARNING!") + string(" ") + message :
		message;

	string const str = formatted(tmp, width, FL_NORMAL_SIZE);
#endif

	fl_set_object_label(message_widget_, str.c_str());
	FL_COLOR const label_color = warning_posted_ ? FL_RED : FL_LCOL;
	fl_set_object_lcol(message_widget_, label_color);

	if (!message_widget_->visible)
		fl_show_object(message_widget_);
}


namespace {

FormDialogView * GetForm(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->form && ob->form->u_vdata);
	FormDialogView * ptr =
		static_cast<FormDialogView *>(ob->form->u_vdata);
	return ptr;
}

} // namespace anon


extern "C" {

void C_FormDialogView_ApplyCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->dialog().ApplyButton();
}


void C_FormDialogView_OKCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->dialog().OKButton();
}


void C_FormDialogView_CancelCB(FL_OBJECT * ob, long)
{
	FormDialogView * form = GetForm(ob);
	form->dialog().CancelButton();
}


void C_FormDialogView_RestoreCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->dialog().RestoreButton();
}


void C_FormDialogView_InputCB(FL_OBJECT * ob, long d)
{
	GetForm(ob)->InputCB(ob, d);
}


static int C_WMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	lyx::Assert(form && form->u_vdata);
	FormDialogView * ptr = static_cast<FormDialogView *>(form->u_vdata);
	ptr->dialog().CancelButton();
	return FL_CANCEL;
}

static int C_PrehandlerCB(FL_OBJECT * ob, int event,
			  FL_Coord, FL_Coord, int key, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.
	lyx::Assert(ob);

	// Don't Assert this one, as it can happen quite naturally when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if (!ob->form) return 0;

	FormDialogView * ptr =
		static_cast<FormDialogView *>(ob->form->u_vdata);

	if (ptr)
		ptr->PrehandlerCB(ob, event, key);

	return 0;
}

} // extern "C"
