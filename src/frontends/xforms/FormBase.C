/**
 * \file FormBase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormBase.h"

#include "ControlButtons.h"
#include "xformsBC.h"
#include "xforms_resize.h"
#include "Tooltips.h"

#include "support/LAssert.h"
#include "support/filetools.h" //  LibFileSearch

#include FORMS_H_LOCATION

extern "C" {

// This should be in forms.h but isn't
void fl_hide_tooltip();

// Callback function invoked by xforms when the dialog is closed by the
// window manager
static int C_WMHideCB(FL_FORM * form, void *);

// Callback function invoked by the xforms pre- and post-handler routines
static int C_PrehandlerCB(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

} // extern "C"


FormBase::FormBase(string const & t, bool allowResize)
	: ViewBase(), minw_(0), minh_(0), allow_resize_(allowResize),
	  title_(t), icon_pixmap_(0), icon_mask_(0),
	  tooltips_(new Tooltips())
{}


FormBase::~FormBase()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);

	delete tooltips_;
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


xformsBC & FormBase::bc()
{
	return static_cast<xformsBC &>(getController().bc());
	// return dynamic_cast<GUIbc &>(controller_ptr_->bc());
}


void FormBase::prepare_to_show()
{
	double const scale = scale_to_fit_tabs(form());
	if (scale > 1.001)
		scale_form(form(), scale);

	bc().refresh();

	// work around dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_form_atclose(form(), C_WMHideCB, 0);

	// set the title for the minimized form
	if (!getController().IconifyWithMain())
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


void FormBase::show()
{
	// build() is/should be called from the controller, so form() should
	// always exist.
	lyx::Assert(form());

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
		int const iconify_policy =
			getController().IconifyWithMain() ? FL_TRANSIENT : 0;

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     iconify_policy,
			     maximize_title.c_str());
	}

	// For some strange reason known only to xforms, the tooltips can only
	// be set on a form that is already visible...
	tooltips().set();
}


void FormBase::hide()
{
	// Does no harm if none is visible and ensures that the tooltip form
	// is hidden should the dialog be closed from the keyboard.
	fl_hide_tooltip();

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
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_PrehandlerCB);
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


namespace {

FormBase * GetForm(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->form && ob->form->u_vdata);
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
	lyx::Assert(form && form->u_vdata);
	FormBase * ptr = static_cast<FormBase *>(form->u_vdata);
	ptr->getController().CancelButton();
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

	FormBase * ptr = static_cast<FormBase *>(ob->form->u_vdata);

	if (ptr)
		ptr->PrehandlerCB(ob, event, key);

	return 0;
}

} // extern "C"
