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
#include "xform_macros.h"

C_RETURNCB(FormBase, WMHideCB)
C_GENERICCB(FormBase, ApplyCB)
C_GENERICCB(FormBase, CancelCB)
C_GENERICCB(FormBase, InputCB)
C_GENERICCB(FormBase, OKCB)

FormBase::FormBase(LyXView * lv, Dialogs * d, string const & t)
	: lv_(lv), d_(d), u_(0), h_(0), title(t), dialogIsOpen(false)
{}


void FormBase::show()
{
	if (!form()) {
		build();
		fl_set_form_atclose(form(),
				    C_FormBaseWMHideCB, 0);
	}

	fl_freeze_form( form() );
	update();  // make sure its up-to-date
	fl_unfreeze_form( form() );

	dialogIsOpen = true;
	if (form()->visible) {
		fl_raise_form(form());
	} else {
		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_TRANSIENT,
			     title.c_str());
		u_ = d_->updateBufferDependent.
		         connect(slot(this, &FormBase::update));
		h_ = d_->hideBufferDependent.
		         connect(slot(this, &FormBase::hide));
	}
}


void FormBase::hide()
{
	if (form() && form()->visible) {
		fl_hide_form(form());
		u_.disconnect();
		h_.disconnect();
	}

	// free up the dialog for another inset
	dialogIsOpen = false;
	clearStore();
}


int FormBase::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormBase * pre = static_cast<FormBase*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormBase::ApplyCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->apply();
}


void FormBase::CancelCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->hide();
}


void FormBase::InputCB(FL_OBJECT * ob, long data )
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->input( data );
}


void FormBase::OKCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}
