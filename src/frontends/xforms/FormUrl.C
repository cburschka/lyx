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
#include "FormUrl.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_url.h"
#include "lyxfunc.h"

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Url")), dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showUrl.connect(slot(this, &FormUrl::showInset));
	d->createUrl.connect(slot(this, &FormUrl::createInset));
}


FormUrl::~FormUrl()
{
	delete dialog_;
}


void FormUrl::build()
{
	dialog_ = build_url();
}


FL_FORM * const FormUrl::form() const
{
	if( dialog_ && dialog_->form_url )
		return dialog_->form_url;
	else
		return 0;
}


void FormUrl::update()
{
	static int ow = -1, oh;

	if (ow < 0) {
		ow = dialog_->form_url->w;
		oh = dialog_->form_url->h;

		fl_set_form_minsize(dialog_->form_url, ow, oh);
		fl_set_form_maxsize(dialog_->form_url, 2*ow, oh);
	}

	fl_freeze_form( dialog_->form_url );

	fl_set_input(dialog_->url,  params.getContents().c_str());
	fl_set_input(dialog_->name, params.getOptions().c_str());

	if ( params.getCmdName() == "url" )
		fl_set_button(dialog_->radio_html, 0);
	else
		fl_set_button(dialog_->radio_html, 1);

	if( lv_->buffer()->isReadonly() ) {
		fl_deactivate_object( dialog_->url );
		fl_deactivate_object( dialog_->name );
		fl_deactivate_object( dialog_->radio_html );
		fl_deactivate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_INACTIVE );
	} else {
		fl_activate_object( dialog_->url );
		fl_activate_object( dialog_->name );
		fl_activate_object( dialog_->radio_html );
		fl_activate_object( dialog_->ok );
		fl_set_object_lcol( dialog_->ok, FL_BLACK );
	}

	fl_unfreeze_form( dialog_->form_url );
}


void FormUrl::apply()
{
	if( lv_->buffer()->isReadonly() ) return;

	params.setContents( fl_get_input(dialog_->url) );
	params.setOptions( fl_get_input(dialog_->name) );

	if (fl_get_button(dialog_->radio_html))
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");

	if( inset_ != 0 )
	{
		// Only update if contents have changed
		if( params.getCmdName()  != inset_->getCmdName()  ||
		    params.getContents() != inset_->getContents() ||
		    params.getOptions()  != inset_->getOptions() ) {
			inset_->setParams( params );
			lv_->view()->updateInset( inset_, true );
		}
	} else {
		lv_->getLyXFunc()->Dispatch( LFUN_INSERT_URL,
					     params.getAsString().c_str() );
	}
}
