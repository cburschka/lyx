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
#include "FormUrl.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_url.h"
#include "lyxfunc.h"

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Url")), minh(0), minw(0), dialog_(0)
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

	// XFORMS bug workaround
	// Define the min/max dimensions. Actually applied in update()
	minw = form()->w; minh = form()->h;
}


FL_FORM * FormUrl::form() const
{
	if ( dialog_ ) return dialog_->form;
	return 0;
}


void FormUrl::update()
{
	fl_set_form_minsize(form(), minw, minh);
	fl_set_form_maxsize(form(), 2*minw, minh);

	fl_set_input(dialog_->url,  params.getContents().c_str());
	fl_set_input(dialog_->name, params.getOptions().c_str());

	if ( params.getCmdName() == "url" )
		fl_set_button(dialog_->radio_html, 0);
	else
		fl_set_button(dialog_->radio_html, 1);

	if ( lv_->buffer()->isReadonly() ) {
		fl_deactivate_object( dialog_->url );
		fl_deactivate_object( dialog_->name );
		fl_deactivate_object( dialog_->radio_html );
		fl_deactivate_object( dialog_->button_ok );
		fl_set_object_lcol( dialog_->button_ok, FL_INACTIVE );
	} else {
		fl_activate_object( dialog_->url );
		fl_activate_object( dialog_->name );
		fl_activate_object( dialog_->radio_html );
		fl_activate_object( dialog_->button_ok );
		fl_set_object_lcol( dialog_->button_ok, FL_BLACK );
	}
}


void FormUrl::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	params.setContents(fl_get_input(dialog_->url));
	params.setOptions(fl_get_input(dialog_->name));

	if (fl_get_button(dialog_->radio_html))
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_INSERT_URL,
					    params.getAsString());
	}
}
