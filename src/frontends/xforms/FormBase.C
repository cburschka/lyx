/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
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
#include "Tooltips.h"
#include "xforms_helpers.h" // formatted

extern "C" {

// Callback function invoked by xforms when the dialog is closed by the
// window manager
static int C_FormBaseWMHideCB(FL_FORM * form, void *);

// Use this to diaplay feedback messages or to trigger an input event on paste
// with the middle mouse button
static int C_FormBasePrehandler(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord, int key, void *);

} // extern "C"


FormBase::FormBase(ControlButtons & c, string const & t, bool allowResize)
	: ViewBC<xformsBC>(c), minw_(0), minh_(0), allow_resize_(allowResize),
	  title_(t), warning_posted_(false), tooltip_level_(NO_TOOLTIP)

{
	tooltip_ = new Tooltips;
	tooltip_->getTooltip.connect(SigC::slot(this, &FormBase::getTooltip));
}


FormBase::~FormBase()
{
	delete tooltip_;
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
		if (!allow_resize_)
			fl_set_form_maxsize(form(), minw_, minh_);

		fl_show_form(form(),
			FL_PLACE_MOUSE | FL_FREE_SIZE,
			(controller_.IconifyWithMain() ? FL_TRANSIENT : 0),
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
void FormBase::FeedbackCB(FL_OBJECT * ob, int event)
{
	lyx::Assert(ob);

	switch (event) {
	case FL_ENTER:
		warning_posted_ = false;
		feedback(ob);
		break;

	case FL_LEAVE:
		if (!warning_posted_)
			clear_feedback();
		break;

	default:
		break;
	}
}


void FormBase::setTooltipHandler(FL_OBJECT * ob)
{
	tooltip_->activateTooltip(ob);
}


string FormBase::getTooltip(FL_OBJECT const * ob)
{
	lyx::Assert(ob);

	switch (tooltip_level_) {
	case VERBOSE_TOOLTIP: 
	{
		string str = getVerboseTooltip(ob);
		if (!str.empty())
			return formatted(_(str), 400);
		// else, fall through
	}
	
	case MINIMAL_TOOLTIP:
		return getMinimalTooltip(ob);
		
	case NO_TOOLTIP:
	default:
		return string();
	}
	
}
		

/// Fill the tooltips chooser with the standard descriptions
void FormBase::fillTooltipChoice(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->objclass == FL_CHOICE);

	fl_clear_choice(ob);
	fl_addto_choice(ob, _(" None | Normal | Verbose "));

	switch(tooltip_level_){
	case NO_TOOLTIP:
		fl_set_choice(ob, 1);
		break;
	case MINIMAL_TOOLTIP:
		fl_set_choice(ob, 2);
		break;
	case VERBOSE_TOOLTIP:
		fl_set_choice(ob, 3);
		break;
	}
}


void FormBase::setTooltipLevel(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->objclass == FL_CHOICE &&
		    fl_get_choice_maxitems(ob) == 3);

	switch(fl_get_choice(ob)){
	case 1:
		tooltip_level_ = NO_TOOLTIP;
		break;
	case 2:
		tooltip_level_ = MINIMAL_TOOLTIP;
		break;
	case 3:
		tooltip_level_ = VERBOSE_TOOLTIP;
		break;
	}
}


void FormBase::setPrehandler(FL_OBJECT * ob)
{
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_FormBasePrehandler);
}


void FormBase::setWarningPosted(bool warning)
{
	warning_posted_ = warning;
}


namespace {

FormBase * GetForm(FL_OBJECT * ob)
{
	lyx::Assert(ob && ob->form && ob->form->u_vdata);
	FormBase * pre = static_cast<FormBase *>(ob->form->u_vdata);
	return pre;
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


static int C_FormBaseWMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	lyx::Assert(form && form->u_vdata);
	FormBase * pre = static_cast<FormBase *>(form->u_vdata);
	pre->CancelButton();
	return FL_CANCEL;
}


static int C_FormBasePrehandler(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord, int key, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.
	lyx::Assert(ob);

	// Don't Assert this one, as it can happen quite naturally when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if (!ob->form) return 0;

	FormBase * pre = static_cast<FormBase *>(ob->form->u_vdata);
	if (!pre) return 0;

	if (event == FL_PUSH && key == 2 && ob->objclass == FL_INPUT) {
		// Trigger an input event when pasting in an xforms input object
		// using the middle mouse button.
		pre->InputCB(ob, 0);

	} else if (event == FL_ENTER || event == FL_LEAVE){
		// Post feedback as the mouse enters the object,
		// remove it as the mouse leaves.
		pre->FeedbackCB(ob, event);
	}

	return 0;
}
 
} // extern "C"
