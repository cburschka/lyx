// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormBase.h"
#include "xformsBC.h"
#include "support/LAssert.h"

extern "C" int C_FormBaseWMHideCB(FL_FORM * form, void *);


FormBase::FormBase(ControlButtons & c, string const & t)
	: ViewBC<xformsBC>(c), minw_(0), minh_(0), title_(t)
{}


void FormBase::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


void FormBase::show()
{
	if (!form()) {
		build();

		bc().refresh();
 
		// work around dumb xforms sizing bug
		minw_ = form()->w;
		minh_ = form()->h;

		fl_set_form_atclose(form(), C_FormBaseWMHideCB, 0);
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
		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE, 0,
			     title_.c_str());
	}
}


void FormBase::hide()
{
	if (form() && form()->visible)
		fl_hide_form(form());
}


void FormBase::InputCB(FL_OBJECT * ob, long data)
{
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
	FormBase * pre = static_cast<FormBase *>(ob->form->u_vdata);
	return pre;
}

} // namespace anon


extern "C"
int C_FormBaseWMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	lyx::Assert(form && form->u_vdata);
	FormBase * pre = static_cast<FormBase *>(form->u_vdata);
	pre->CancelButton();
	return FL_CANCEL;
}


extern "C" void C_FormBaseApplyCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->ApplyButton();
}


extern "C" void C_FormBaseOKCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->OKButton();
}


extern "C" void C_FormBaseCancelCB(FL_OBJECT * ob, long)
{
	FormBase * form = GetForm(ob);
	form->CancelButton();
}


extern "C" void C_FormBaseRestoreCB(FL_OBJECT * ob, long)
{
	GetForm(ob)->RestoreButton();
}


extern "C" void C_FormBaseInputCB(FL_OBJECT * ob, long d)
{
	GetForm(ob)->InputCB(ob, d);
}
