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
#include "gettext.h"
#include FORMS_H_LOCATION
#include "BufferView.h"
#include "Dialogs.h"
#include "FormUrl.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_url.h"
#include "lyxfunc.h"
#include "xform_macros.h"
#include "insets/insetcommand.h"
#include "insets/inseturl.h"
#include "support/filetools.h"

#ifdef __GNUG__
#pragma implementation
#endif

C_RETURNCB(FormUrl, WMHideCB)
C_GENERICCB(FormUrl, OKCB)
C_GENERICCB(FormUrl, CancelCB)

FormUrl::FormUrl(LyXView * lv, Dialogs * d)
	: dialog_(0), lv_(lv), d_(d), u_(0), h_(0), ih_(0),
	  inset_(0), dialogIsOpen(false)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showUrl.connect(slot(this, &FormUrl::showInset));
	d->createUrl.connect(slot(this, &FormUrl::createInset));
	params = new InsetCommandParams();
}


FormUrl::~FormUrl()
{
	free();
	delete params;
}


void FormUrl::build()
{
	dialog_ = build_url();
}


void FormUrl::showInset( InsetUrl * inset )
{
	if( dialogIsOpen || inset == 0 ) return;

	inset_ = inset;
	ih_ = inset_->hide.connect(slot(this, &FormUrl::hide));

	(*params) = inset->params();
	show();
}


void FormUrl::createInset( string const & arg )
{
	if( dialogIsOpen ) return;

	params->setFromString( arg );
	show();
}


void FormUrl::show()
{
	if (!dialog_) {
		build();
		fl_set_form_atclose(dialog_->form_url,
				    C_FormUrlWMHideCB, 0);
	}

	update();  // make sure its up-to-date

	dialogIsOpen = true;
	if (dialog_->form_url->visible) {
		fl_raise_form(dialog_->form_url);
	} else {
		fl_show_form(dialog_->form_url,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_TRANSIENT,
			     _("Url"));
		u_ = d_->updateBufferDependent.
		         connect(slot(this, &FormUrl::update));
		h_ = d_->hideBufferDependent.
		         connect(slot(this, &FormUrl::hide));
	}
}


void FormUrl::update()
{
	fl_set_input(dialog_->url,  params->getContents().c_str());
	fl_set_input(dialog_->name, params->getOptions().c_str());

	if ( params->getCmdName() == "url" )
		fl_set_button(dialog_->radio_html, 0);
	else
		fl_set_button(dialog_->radio_html, 1);

	static int ow = -1, oh;

	if (ow < 0) {
		ow = dialog_->form_url->w;
		oh = dialog_->form_url->h;
	}

	fl_set_form_minsize(dialog_->form_url, ow, oh);
	fl_set_form_maxsize(dialog_->form_url, 2*ow, oh);
}


void FormUrl::apply()
{
	if( lv_->buffer()->isReadonly() ) return;

	params->setContents( fl_get_input(dialog_->url) );
	params->setOptions( fl_get_input(dialog_->name) );

	if (fl_get_button(dialog_->radio_html))
		params->setCmdName("htmlurl");
	else
		params->setCmdName("url");

	if( inset_ != 0 )
	{
		inset_->setParams( *params );
		lv_->view()->updateInset( inset_, true );
	} else {
		lv_->getLyXFunc()->Dispatch( LFUN_INSERT_URL,
					     params->getAsString().c_str() );
	}
}


void FormUrl::hide()
{
	if (dialog_
	    && dialog_->form_url
	    && dialog_->form_url->visible) {
		fl_hide_form(dialog_->form_url);
		u_.disconnect();
		h_.disconnect();
	}

	// free up the dialog for another inset
	inset_ = 0;
	ih_.disconnect();
	dialogIsOpen = false;
}


void FormUrl::free()
{
	// we don't need to delete u and h here because
	// hide() does that after disconnecting.
	if (dialog_) {
		if (dialog_->form_url
		    && dialog_->form_url->visible) {
			hide();
		}
		fl_free_form(dialog_->form_url);
		delete dialog_;
		dialog_ = 0;
	}
}


int FormUrl::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormUrl * pre = static_cast<FormUrl*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormUrl::OKCB(FL_OBJECT * ob, long)
{
	FormUrl * pre = static_cast<FormUrl*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}


void FormUrl::CancelCB(FL_OBJECT * ob, long)
{
	FormUrl * pre = static_cast<FormUrl*>(ob->form->u_vdata);
	pre->hide();
}
