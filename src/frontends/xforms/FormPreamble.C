/**
 * \file FormPreamble.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "form_preamble.h"
#include "FormPreamble.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "gettext.h"
#include "xforms_helpers.h"

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif

FormPreamble::FormPreamble(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("LaTeX preamble"))
{
    // let the popup be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showPreamble.connect(slot(this, &FormPreamble::show));
}


FL_FORM * FormPreamble::form() const
{
    if (dialog_.get()) return dialog_->form;
    return 0;
}


void FormPreamble::build()
{
   dialog_.reset(build_preamble());
   // Workaround dumb xforms sizing bug
   minw_ = form()->w;
   minh_ = form()->h;
   
   fl_set_input_return(dialog_->input_preamble, FL_RETURN_CHANGED);
   // Manage the ok, apply and cancel/close buttons
   bc().setOK(dialog_->button_ok);
   bc().setApply(dialog_->button_apply);
   bc().setCancel(dialog_->button_cancel);
   bc().addReadOnly(dialog_->input_preamble);
   bc().refresh();
}


void FormPreamble::apply()
{
   if (!lv_->view()->available() || !dialog_.get())
     return;
   
   // is this needed?:
   // lv_->view()->update(BufferView::SELECT | BufferView::FITCUR | BufferView::CHANGE);
   
   lv_->buffer()->params.preamble = fl_get_input(dialog_->input_preamble);
   lv_->buffer()->markDirty();
   setMinibuffer(lv_, _("LaTeX preamble set"));
}


void FormPreamble::update()
{
   if (!dialog_.get())
     return;

   fl_set_input(dialog_->input_preamble,lv_->buffer()->params.preamble.c_str());

   bool const enable = (! lv_->buffer()->isReadonly());
   setEnabled(dialog_->input_preamble, enable);
   setEnabled(dialog_->button_ok,      enable);
   setEnabled(dialog_->button_apply,   enable);
   
   // need this?
   // bc().readOnly(lv_->buffer()->isReadonly());
}


