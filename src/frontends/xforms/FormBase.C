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

C_RETURNCB (FormBase, WMHideCB)
C_GENERICCB(FormBase, ApplyCB)
C_GENERICCB(FormBase, ApplyHideCB)
C_GENERICCB(FormBase, HideCB)
C_GENERICCB(FormBase, InputCB)


FormBase::FormBase(LyXView * lv, Dialogs * d, BufferDependency bd, string const & t)
	: dialogIsOpen(false), lv_(lv), u_(0), h_(0), title(t)
{
	switch( bd ) {
	case BUFFER_DEPENDENT:
		hSignal_ = &d->hideBufferDependent;
		uSignal_ = &d->updateBufferDependent;
		break;
	case BUFFER_INDEPENDENT:
		hSignal_ = &d->hideAll;
		uSignal_ = 0;
		break;
	}
}

		
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
		if( uSignal_ )
			u_ = uSignal_->connect(slot(this, &FormBase::update));
		h_ = hSignal_->connect(slot(this, &FormBase::hide));
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


void FormBase::ApplyHideCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}


void FormBase::HideCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->hide();
}


void FormBase::InputCB(FL_OBJECT * ob, long data )
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->input( data );
}
