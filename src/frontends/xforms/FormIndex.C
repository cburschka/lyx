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
	delete dialog_;
}


FL_FORM * FormIndex::form() const
{
	if ( dialog_ ) return dialog_->form;
	return 0;
}


void FormIndex::connect()
{
	fl_set_form_maxsize( form(), 2*minw_, minh_ );
	FormCommand::connect();
}
	

void FormIndex::build()
{
	dialog_ = build_index();

#ifdef WITH_WARNINGS
#warning use the buttoncontroller
#endif
	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;
}


void FormIndex::update()
{
	fl_set_input(dialog_->key, params.getContents().c_str());

	if ( lv_->buffer()->isReadonly() ) {
		fl_deactivate_object( dialog_->key );
		fl_deactivate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_INACTIVE );
	} else {
		fl_activate_object( dialog_->key );
		fl_activate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_BLACK );
	}
}


void FormIndex::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	params.setContents(fl_get_input(dialog_->key));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_INDEX_INSERT,
					    params.getAsString());
	}
}
