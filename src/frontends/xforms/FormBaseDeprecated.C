/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormBaseDeprecated.h"
#include "LyXView.h"
#include "support/LAssert.h"
#include "xformsBC.h"
#include "lyxrc.h"

using SigC::slot;

extern "C" {

// Callback function invoked by xforms when the dialog is closed by the
// window manager
static int C_FormBaseDeprecatedWMHideCB(FL_FORM *, void *);
 
} // extern "C"


FormBaseDeprecated::FormBaseDeprecated(LyXView * lv, Dialogs * d,
				       string const & t, bool allowResize)
	: lv_(lv), d_(d), h_(0), r_(0), title_(t),
	  minw_(0), minh_(0), allow_resize_(allowResize)
{
	lyx::Assert(lv && d);
}


void FormBaseDeprecated::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


void FormBaseDeprecated::connect()
{
	fl_set_form_minsize(form(), minw_, minh_);
	r_ = Dialogs::redrawGUI.connect(slot(this,&FormBaseDeprecated::redraw));
}


void FormBaseDeprecated::disconnect()
{
	h_.disconnect();
	r_.disconnect();
}


void FormBaseDeprecated::show()
{
	if (!form()) {
		build();

		bc().refresh();
 
		// work around dumb xforms sizing bug
		minw_ = form()->w;
		minh_ = form()->h;

		fl_set_form_atclose(form(),
				    C_FormBaseDeprecatedWMHideCB, 0);
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
		connect();

		// calls to fl_set_form_minsize/maxsize apply only to the next
		// fl_show_form(), so this comes first.
		fl_set_form_minsize(form(), minw_, minh_);
		if (!allow_resize_)
			fl_set_form_maxsize(form(), minw_, minh_);

		fl_show_form(form(),
			FL_PLACE_MOUSE | FL_FREE_SIZE,
			(lyxrc.dialogs_iconify_with_main ? FL_TRANSIENT : 0),
			title_.c_str());
	}
}


void FormBaseDeprecated::hide()
{
	if (form() && form()->visible) {
		// some dialogs might do things to the form first
		// such as the nested tabfolder problem in Preferences
		disconnect();
		fl_hide_form(form());
	}
}


int FormBaseDeprecated::WMHideCB(FL_FORM * form, void *)
{
	lyx::Assert(form);
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(form->u_vdata);
	lyx::Assert(pre);
	pre->hide();
	pre->bc().hide();
	return FL_CANCEL;
}


void FormBaseDeprecated::ApplyCB(FL_OBJECT * ob, long)
{
	lyx::Assert(ob && ob->form);
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(ob->form->u_vdata);
	lyx::Assert(pre);
	pre->apply();
	pre->bc().apply();
}


void FormBaseDeprecated::OKCB(FL_OBJECT * ob, long)
{
	lyx::Assert(ob && ob->form);
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(ob->form->u_vdata);
	lyx::Assert(pre);
	pre->ok();
	pre->bc().ok();
}


void FormBaseDeprecated::CancelCB(FL_OBJECT * ob, long)
{
	lyx::Assert(ob && ob->form);
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(ob->form->u_vdata);
	lyx::Assert(pre);
	pre->cancel();
	pre->bc().cancel();
}


void FormBaseDeprecated::InputCB(FL_OBJECT * ob, long data)
{
	lyx::Assert(ob && ob->form);
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(ob->form->u_vdata);
	lyx::Assert(pre);

	// It is possible to set the choice to 0 when using the
	// keyboard shortcuts. This work-around deals with the problem.
	if (ob && ob->objclass == FL_CHOICE && fl_get_choice(ob) < 1) {
		fl_set_choice(ob, 1);
	}

	pre->bc().valid(pre->input(ob, data));
}


void FormBaseDeprecated::RestoreCB(FL_OBJECT * ob, long)
{
	lyx::Assert(ob && ob->form);
	FormBaseDeprecated * pre =
		static_cast<FormBaseDeprecated*>(ob->form->u_vdata);
	lyx::Assert(pre);
	pre->bc().restore();
	pre->restore();
}


FormBaseBI::FormBaseBI(LyXView * lv, Dialogs * d, string const & t,
		       bool allowResize)
	: FormBaseDeprecated(lv, d, t, allowResize)
{}


void FormBaseBI::connect()
{
	h_ = d_->hideAll.connect(slot(this, &FormBaseBI::hide));
	FormBaseDeprecated::connect();
}


FormBaseBD::FormBaseBD(LyXView * lv, Dialogs * d, string const & t,
		       bool allowResize)
	: FormBaseDeprecated(lv, d, t, allowResize),
	  u_(0)
{}


void FormBaseBD::connect()
{
	u_ = d_->updateBufferDependent.
		 connect(slot(this, &FormBaseBD::updateSlot));
	h_ = d_->hideBufferDependent.
		 connect(slot(this, &FormBaseBD::hide));
	FormBaseDeprecated::connect();
}


void FormBaseBD::disconnect()
{
	u_.disconnect();
	FormBaseDeprecated::disconnect();
}


extern "C" {
	
static int C_FormBaseDeprecatedWMHideCB(FL_FORM * ob, void * d)
{
	return FormBaseDeprecated::WMHideCB(ob, d);
}


void C_FormBaseDeprecatedApplyCB(FL_OBJECT * ob, long d)
{
	FormBaseDeprecated::ApplyCB(ob, d);
}


void C_FormBaseDeprecatedOKCB(FL_OBJECT * ob, long d)
{
	FormBaseDeprecated::OKCB(ob, d);
}


void C_FormBaseDeprecatedCancelCB(FL_OBJECT * ob, long d)
{
	FormBaseDeprecated::CancelCB(ob, d);
}


void C_FormBaseDeprecatedInputCB(FL_OBJECT * ob, long d)
{
		FormBaseDeprecated::InputCB(ob, d);
}


void C_FormBaseDeprecatedRestoreCB(FL_OBJECT * ob, long d)
{
	FormBaseDeprecated::RestoreCB(ob, d);
}

} // extern "C"

