/**
 * \file FormSearch.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "FormSearch.h"
#include "form_search.h"
#include "gettext.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "gettext.h"
#include "lyxfind.h"
#include "debug.h"

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif


FormSearch::FormSearch(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("LyX: Find and Replace"), new NoRepeatedApplyReadOnlyPolicy),
	dialog_(0)
{
    // let the popup be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showSearch.connect(slot(this, &FormSearch::show));
   // perhaps in the future we'd like a
   // "search again" button/keybinding
//    d->searchAgain.connect(slot(this, &FormSearch::FindNext));
}


FormSearch::~FormSearch()
{
   delete dialog_;
}


FL_FORM * FormSearch::form() const
{
    if (dialog_) 
     return dialog_->form;
    return 0;
}

void FormSearch::build()
{
   dialog_ = build_search();
   // Workaround dumb xforms sizing bug
   minw_ = form()->w;
   minh_ = form()->h;
	
   // Manage the ok, apply and cancel/close buttons
   bc_.setCancel(dialog_->button_cancel);
   bc_.addReadOnly(dialog_->input_replace);
   bc_.addReadOnly(dialog_->replace);
   bc_.addReadOnly(dialog_->replaceall);
   bc_.refresh();
}

void FormSearch::update()
{
   if (!dialog_)
     return;

   bc_.readOnly(lv_->buffer()->isReadonly());
}

bool FormSearch::input(FL_OBJECT * obj, long)
{
   if (obj == dialog_->findnext)
     Find();
   else if (obj == dialog_->findprev)
     Find(false);
   else if (obj == dialog_->replace)
     Replace();
   else if (obj == dialog_->replaceall)
     Replace(true);
   
   return 0;
}

void FormSearch::Find(bool const next = true)
{
   bool found = LyXFind(lv_->view(),
			fl_get_input(dialog_->input_search),
			fl_get_button(dialog_->casesensitive),
			fl_get_button(dialog_->matchword),
			next);
   
   if (!found)
     setMinibuffer(lv_, _("String not found!"));
 };


void FormSearch::Replace(bool const all = false)
{
   int replace_count = LyXReplace(lv_->view(),
				  fl_get_input(dialog_->input_search),
				  fl_get_input(dialog_->input_replace),
				  fl_get_button(dialog_->casesensitive),
				  fl_get_button(dialog_->matchword), 
				  true, 
				  all);
				  
   if (replace_count == 0) {
      setMinibuffer(lv_, _("String not found!"));
   } else {
      if (replace_count == 1) {
	 setMinibuffer(lv_, _("String has been replaced."));
      } else {
	 string str = tostr(replace_count);
	 str += _(" strings have been replaced.");
	 setMinibuffer(lv_, str.c_str());
      }
   }
};


