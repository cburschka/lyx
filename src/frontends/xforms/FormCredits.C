/**
 * \file FormCredits.C
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

#include "Dialogs.h"
#include "LyXView.h"
#include "form_credits.h"
#include "FormCredits.h"
#include "xforms_helpers.h"
#include "support/filetools.h"

FormCredits::FormCredits( LyXView * lv, Dialogs * d )
  : FormBaseBI(lv, d, _("Credits"), new OkCancelPolicy), dialog_(0)
{
   // let the dialog be shown
   // This is a permanent connection so we won't bother
   // storing a copy because we won't be disconnecting.
   d->showCredits.connect(slot(this, &FormCredits::show));
}


FormCredits::~FormCredits()
{
   delete dialog_;
}


FL_FORM * FormCredits::form() const
{
   if (dialog_ ) 
     return dialog_->form;
   return 0;
}

// needed for the browser
extern string system_lyxdir;

void FormCredits::build()
{
   dialog_ = build_credits();

   // Workaround dumb xforms sizing bug
   minw_ = form()->w;
   minh_ = form()->h;

   // Manage the cancel/close button
   bc_.setCancel(dialog_->button_cancel);
   bc_.refresh();
		
   /* read the credits into the browser */ 
		
   /* try file LYX_DIR/CREDITS */ 
   string real_file = AddName (system_lyxdir, "CREDITS");
   
   if (!fl_load_browser(dialog_->browser_credits,
			real_file.c_str())) {
      fl_add_browser_line(dialog_->browser_credits,
			  _("ERROR: LyX wasn't able to read"
			    " CREDITS file"));
      fl_add_browser_line(dialog_->browser_credits, "");
      fl_add_browser_line(dialog_->browser_credits,
			  _("Please install correctly to estimate"
			    " the great"));
      fl_add_browser_line(dialog_->browser_credits,
			  _("amount of work other people have done"
			    " for the LyX project."));
   }
}
