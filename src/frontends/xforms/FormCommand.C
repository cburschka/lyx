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
#include "FormCommand.h"
#include "xform_macros.h"

C_RETURNCB(FormCommand, WMHideCB)
C_GENERICCB(FormCommand, ApplyCB)
C_GENERICCB(FormCommand, CancelCB)
C_GENERICCB(FormCommand, InputCB)
C_GENERICCB(FormCommand, OKCB)

FormCommand::FormCommand(LyXView * lv, Dialogs * d, string const & t)
	: lv_(lv), d_(d), inset_(0), u_(0), h_(0), ih_(0),
	  dialogIsOpen(false), title(t)
{}


void FormCommand::showInset( InsetCommand * const inset )
{
	if( dialogIsOpen || inset == 0 ) return;

	inset_ = inset;
	ih_ = inset_->hide.connect(slot(this, &FormCommand::hide));

	params = inset->params();
	show();
}


void FormCommand::createInset( string const & arg )
{
	if( dialogIsOpen ) return;

	params.setFromString( arg );
	show();
}


void FormCommand::show()
{
	if (!form()) {
		build();
		fl_set_form_atclose(form(),
				    C_FormCommandWMHideCB, 0);
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
		         connect(slot(this, &FormCommand::update));
		h_ = d_->hideBufferDependent.
		         connect(slot(this, &FormCommand::hide));
	}
}


void FormCommand::hide()
{
	if (form() && form()->visible) {
		fl_hide_form(form());
		u_.disconnect();
		h_.disconnect();
	}

	// free up the dialog for another inset
	inset_ = 0;
	ih_.disconnect();
	dialogIsOpen = false;
	clearStore();
}


int FormCommand::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormCommand * pre = static_cast<FormCommand*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormCommand::ApplyCB(FL_OBJECT * ob, long)
{
	FormCommand * pre = static_cast<FormCommand*>(ob->form->u_vdata);
	pre->apply();
}


void FormCommand::CancelCB(FL_OBJECT * ob, long)
{
	FormCommand * pre = static_cast<FormCommand*>(ob->form->u_vdata);
	pre->hide();
}


void FormCommand::InputCB(FL_OBJECT * ob, long data )
{
	FormCommand * pre = static_cast<FormCommand*>(ob->form->u_vdata);
	pre->input( data );
}


void FormCommand::OKCB(FL_OBJECT * ob, long)
{
	FormCommand * pre = static_cast<FormCommand*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}
