/**
 * \file FormBase.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormBase.h"
#include "xformsBC.h"
#include "xforms_resize.h"
#include "Tooltips.h"
#include "support/LAssert.h"
#include FORMS_H_LOCATION

extern "C" {

// Callback function invoked by xforms when the dialog is closed by the
// window manager
static int C_WMHideCB(FL_FORM * form, void *);

// Callback function invoked by the xforms pre- and post-handler routines
static int C_PrehandlerCB(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

} // extern "C"


FormBase::FormBase(ControlButtons & c, string const & t, bool allowResize)
	: ViewBC<xformsBC>(c), minw_(0), minh_(0), allow_resize_(allowResize),
	  title_(t), tooltips_(new Tooltips)
{}


FormBase::~FormBase()
{
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


void FormBase::show()
{
	if (!form()) {
		build();
	}

	// use minw_ to flag whether the dialog has ever been shown
	// (Needed now that build() is/should be called from the controller)
	if (minw_ == 0) {
		double const scale = scale_to_fit_tabs(form());
		if (scale > 1.001)
			scale_form(form(), scale);

		bc().refresh();

		// work around dumb xforms sizing bug
		minw_ = form()->w;
		minh_ = form()->h;

		fl_set_form_atclose(form(), C_WMHideCB, 0);
	}

	fl_freeze_form(form());
	update();  // make sure its up-to-date
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

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     (controller_.IconifyWithMain() ? FL_TRANSIENT : 0),
			     title_.c_str());
	}

	tooltips().set();
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
	GetForm(ob)->ApplyButton();
}


void C_FormBaseOKCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->OKButton();
}


void C_FormBaseCancelCB(FL_OBJECT * ob, long)
{
	FormBase * form = GetForm(ob);
	form->CancelButton();
}


void C_FormBaseRestoreCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->RestoreButton();
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
	ptr->CancelButton();
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
