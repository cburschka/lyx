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

C_RETURNCB (FormBase, WMHideCB)
C_GENERICCB(FormBase, ApplyCB)
C_GENERICCB(FormBase, OKCB)
C_GENERICCB(FormBase, CancelCB)
C_GENERICCB(FormBase, InputCB)
C_GENERICCB(FormBase, RestoreCB)


FormBase::FormBase(LyXView * lv, Dialogs * d, string const & t,
		   BufferDependency bd, ChangedBufferAction cba, 
		   ButtonPolicy * bp, char const * close, char const * cancel)
	: dialogIsOpen(false), lv_(lv), bc_(bp, cancel, close),
	  d_(d), bd_(bd), cba_(cba), parent_(0), u_(0), h_(0), title(t), bp_(bp)
{}


FormBase::~FormBase()
{
	delete bp_;
}


void FormBase::show()
{
	if (!form()) {
		build();
		fl_set_form_atclose(form(),
				    C_FormBaseWMHideCB, 0);
	}

	parent_ = lv_->buffer();
	
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
		connect();
	}
}


void FormBase::hide()
{
	if (form() && form()->visible) {
		fl_hide_form(form());
		disconnect();
	}

	// free up the dialog for another inset
	dialogIsOpen = false;
	parent_ = 0;
	clearStore();
}


void FormBase::connect()
{
	switch( bd_ ) {
	case BUFFER_DEPENDENT:
		u_ = d_->updateBufferDependent.
			connect(slot(this, &FormBase::updateOrHide));
		h_ = d_->hideBufferDependent.
			connect(slot(this, &FormBase::hide));
		break;
	case BUFFER_INDEPENDENT:
		h_ = d_->hideAll.connect(slot(this, &FormBase::hide));
		break;
	}
}


void FormBase::disconnect()
{
	u_.disconnect();
	h_.disconnect();
}


void FormBase::updateOrHide()
{
	if( cba_ == UPDATE )
		update();
	else if( parent_ == lv_->buffer() )
		update();
	else
		hide();
}


int FormBase::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormBase * pre = static_cast<FormBase*>(form->u_vdata);
	pre->hide();
	pre->bc_.hide();
	return FL_CANCEL;
}


void FormBase::ApplyCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->apply();
	pre->bc_.apply();
}


void FormBase::OKCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->ok();
	pre->bc_.ok();
}


void FormBase::CancelCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->cancel();
	pre->bc_.cancel();
}


void FormBase::InputCB(FL_OBJECT * ob, long data )
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->bc_.valid( pre->input( ob, data ) );
}


void FormBase::RestoreCB(FL_OBJECT * ob, long)
{
	FormBase * pre = static_cast<FormBase*>(ob->form->u_vdata);
	pre->restore();
	pre->bc_.undoAll();
}
