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
#include "xform_macros.h"
#include "support/LAssert.h"
//#include "debug.h"

// The current scheme muddles debugging.
// Can we please use some other means to create these functions?
// I really don't like to use the preprossessor for this.
// My suggestion: First of all move these functions into their own
// file (that can be included here if wanted, and use m4 to expand
// that file. So create a m4 function to do the expansion, a file
// that contains the calls to to this function and a script to run
// it and create the C++ file with the expanded functions. (Lgb)
// Possible startoff point:
// define([C_RETURNCB],[extern "C" int C_$1$2(FL_FORM * ob, void * d) { return $1::$2(ob, d); }])

C_RETURNCB (FormBase, WMHideCB)
C_GENERICCB(FormBase, ApplyCB)
C_GENERICCB(FormBase, OKCB)
C_GENERICCB(FormBase, CancelCB)
C_GENERICCB(FormBase, InputCB)
C_GENERICCB(FormBase, RestoreCB)


FormBase::FormBase(LyXView * lv, Dialogs * d, string const & t,
		   ButtonPolicy * bp, char const * close, char const * cancel)
	: lv_(lv), bc_(bp, cancel, close), d_(d), h_(0), r_(0), title(t),
	  bp_(bp), minw_(0), minh_(0)
{
	Assert(lv && d && bp);
}


FormBase::~FormBase()
{
	//lyxerr << "bp_[" << bp_ << "]" << endl;
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
	} else {
		// calls to fl_set_form_minsize/maxsize apply only to the next
		// fl_show_form(), so connect() comes first.
		connect();
		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_TRANSIENT,
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
	pre->bc_.valid(pre->input(ob, data));
}


void FormBase::RestoreCB(FL_OBJECT * ob, long)
{
	Assert(ob && ob->form);
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	Assert(ob);
	pre->restore();
	pre->bc_.undoAll();
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
