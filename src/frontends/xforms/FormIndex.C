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


#include "gettext.h"
#include "Dialogs.h"
#include "FormIndex.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_index.h"
#include "lyxfunc.h"

FormIndex::FormIndex(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Index")), dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showIndex.connect(slot(this, &FormIndex::showInset));
	d->createIndex.connect(slot(this, &FormIndex::createInset));
}


FormIndex::~FormIndex()
{
	free();
	delete dialog_;
}


void FormIndex::build()
{
	dialog_ = build_index();
}


FL_FORM * const FormIndex::form() const
{
	if( dialog_ && dialog_->form_index )
		return dialog_->form_index;
	else
		return 0;
}


void FormIndex::update()
{
	static int ow = -1, oh;

	if (ow < 0) {
		ow = dialog_->form_index->w;
		oh = dialog_->form_index->h;

		fl_set_form_minsize(dialog_->form_index, ow, oh);
		fl_set_form_maxsize(dialog_->form_index, 2*ow, oh);
	}

	fl_freeze_form( dialog_->form_index );

	fl_set_input(dialog_->key, params.getContents().c_str());

	if( lv_->buffer()->isReadonly() ) {
		fl_deactivate_object( dialog_->key );
		fl_deactivate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_INACTIVE );
	} else {
		fl_activate_object( dialog_->key );
		fl_activate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_BLACK );
	}

	fl_unfreeze_form( dialog_->form_index );
}


void FormIndex::apply()
{
	if( lv_->buffer()->isReadonly() ) return;

	params.setContents( fl_get_input(dialog_->key) );

	if( inset_ != 0 )
	{
		// Only update if contents have changed
		if( params.getContents() != inset_->getContents() ) {
			inset_->setParams( params );
			lv_->view()->updateInset( inset_, true );
		}
	} else {
		lv_->getLyXFunc()->Dispatch( LFUN_INDEX_INSERT,
		 			     params.getAsString().c_str() );
	}
}
