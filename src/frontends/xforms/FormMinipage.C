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
/* FormMinipage.C
 * FormMinipage Interface Class Implementation
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormMinipage.h"
#include "form_minipage.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "insets/insetminipage.h"
#include "support/lstrings.h"

FormMinipage::FormMinipage(LyXView * lv, Dialogs * d)
	: FormInset(lv, d, _("Minipage Options")),
	  inset_(0)
{
    // let the dialog be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showMinipage.connect(SigC::slot(this, &FormMinipage::showInset));
    d->updateMinipage.connect(SigC::slot(this, &FormMinipage::updateInset));
}


FL_FORM * FormMinipage::form() const
{
    if (dialog_.get())
	return dialog_->form;
    return 0;
}


void FormMinipage::connect()
{
    bc().valid(true);
    FormBaseBD::connect();
}


void FormMinipage::showInset(InsetMinipage * inset)
{
    if (inset == 0) return;

    // If connected to another inset, disconnect from it.
    if (inset_ != inset) {
	ih_.disconnect();
	ih_ = inset->hideDialog.connect(SigC::slot(this, &FormMinipage::hide));
	inset_ = inset;
    }

    show();
}


void FormMinipage::updateInset(InsetMinipage * inset)
{
    if (inset == 0 || inset_ == 0) return;

    // If connected to another inset, disconnect from it.
    if (inset_ != inset) {
	ih_.disconnect();
	ih_ = inset->hideDialog.connect(SigC::slot(this, &FormMinipage::hide));
	inset_ = inset;
    }

    update();
}

void FormMinipage::build()
{
    dialog_.reset(build_minipage());

    // Workaround dumb xforms sizing bug
    minw_ = form()->w;
    minh_ = form()->h;

    fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);
    fl_set_input_return(dialog_->input_widthp, FL_RETURN_CHANGED);

    // Manage the ok, apply and cancel/close buttons
    bc().setOK(dialog_->button_ok);
    bc().setApply(dialog_->button_apply);
    bc().setCancel(dialog_->button_cancel);
    bc().refresh();
}


void FormMinipage::apply()
{
#if 0
    int ysize = int(fl_get_slider_value(dialog_->slider_columns) + 0.5);
    int xsize = int(fl_get_slider_value(dialog_->slider_rows) + 0.5);

    string tmp = tostr(xsize) + " " + tostr(ysize);
    lv_->getLyXFunc()->Dispatch(LFUN_INSET_TABULAR, tmp);
#endif
}


void FormMinipage::update()
{
    if (!inset_)
	return;
    fl_set_input(dialog_->input_width, inset_->width().c_str());
    fl_set_input(dialog_->input_widthp, tostr(inset_->widthp()).c_str());
		 
    switch (inset_->pos()) {
    case InsetMinipage::top:
	fl_set_button(dialog_->radio_top, 1);
	break;
    case InsetMinipage::center:
	fl_set_button(dialog_->radio_middle, 1);
	break;
    case InsetMinipage::bottom:
	fl_set_button(dialog_->radio_bottom, 1);
	break;
    }
    bc().readOnly(lv_->buffer()->isReadonly());
}
