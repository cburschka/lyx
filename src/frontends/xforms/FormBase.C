// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormBase.h"
#include "LyXView.h"
#include "support/LAssert.h"
//#include "debug.h"

extern "C" int C_FormBaseWMHideCB(FL_FORM * ob, void * d)
{
	return FormBase::WMHideCB(ob, d);
}
extern "C" void C_FormBaseApplyCB(FL_OBJECT * ob, long d)
{
	FormBase::ApplyCB(ob, d);
}
extern "C" void C_FormBaseOKCB(FL_OBJECT * ob, long d)
{
	FormBase::OKCB(ob, d);
}
extern "C" void C_FormBaseCancelCB(FL_OBJECT * ob, long d)
{
	FormBase::CancelCB(ob, d);
}
extern "C" void C_FormBaseInputCB(FL_OBJECT * ob, long d)
{
	FormBase::InputCB(ob, d);
}
extern "C" void C_FormBaseRestoreCB(FL_OBJECT * ob, long d)
{
	FormBase::RestoreCB(ob, d);
}


FormBase::FormBase(LyXView * lv, Dialogs * d, string const & t,
		   ButtonPolicy * bp, char const * close, char const * cancel)
	: lv_(lv), bc_(bp, cancel, close), d_(d), h_(0), r_(0), title(t),
	  bp_(bp), minw_(0), minh_(0)
{
	Assert(lv && d && bp);
}


FormBase::~FormBase()
{
	delete bp_;
}


void FormBase::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
}


void FormBase::connect()
{
	fl_set_form_minsize(form(), minw_, minh_);
	r_ = Dialogs::redrawGUI.connect(slot(this, &FormBase::redraw));
}


void FormBase::disconnect()
{
	h_.disconnect();
	r_.disconnect();
}


void FormBase::show()
{
	if (!form()) {
		build();
		fl_set_form_atclose(form(),
				    C_FormBaseWMHideCB, 0);
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
		// fl_show_form(), so connect() comes first.
		connect();
		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE, 0,
			     title.c_str());
	}
}


void FormBase::hide()
{
	if (form() && form()->visible) {
		// some dialogs might do things to the form first
		// such as the nested tabfolder problem in Preferences
		disconnect();
		fl_hide_form(form());
	}
}


int FormBase::WMHideCB(FL_FORM * form, void *)
{
	Assert(form);
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormBase * pre = static_cast<FormBase*>(form->u_vdata);
	Assert(pre);
	pre->hide();
	pre->bc_.hide();
	return FL_CANCEL;
}


void FormBase::ApplyCB(FL_OBJECT * ob, long)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(pre);
	pre->apply();
	pre->bc_.apply();
}


void FormBase::OKCB(FL_OBJECT * ob, long)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(pre);
	pre->ok();
	pre->bc_.ok();
}


void FormBase::CancelCB(FL_OBJECT * ob, long)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(pre);
	pre->cancel();
	pre->bc_.cancel();
}


void FormBase::InputCB(FL_OBJECT * ob, long data)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(ob);
	pre->bc_.valid(pre->input(ob, data), ob);
}


void FormBase::RestoreCB(FL_OBJECT * ob, long)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(ob);
	pre->bc_.undoAll();
	pre->restore();
}


FormBaseBI::FormBaseBI(LyXView * lv, Dialogs * d, string const & t,
		       ButtonPolicy * bp,
		       char const * close, char const * cancel)
	: FormBase(lv, d, t, bp, close, cancel)
{}


void FormBaseBI::connect()
{
	h_ = d_->hideAll.connect(slot(this, &FormBaseBI::hide));
	FormBase::connect();
}


FormBaseBD::FormBaseBD(LyXView * lv, Dialogs * d, string const & t,
		       ButtonPolicy * bp,
		       char const * close, char const * cancel)
	: FormBase(lv, d, t, bp, close, cancel),
	  u_(0)
{}


void FormBaseBD::connect()
{
	u_ = d_->updateBufferDependent.
		 connect(slot(this, &FormBaseBD::updateSlot));
	h_ = d_->hideBufferDependent.
		 connect(slot(this, &FormBaseBD::hide));
	FormBase::connect();
}


void FormBaseBD::disconnect()
{
	u_.disconnect();
	FormBase::disconnect();
}
