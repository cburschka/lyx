/**
 * \file FormSearch.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "searchdlgimpl.h"
#include "FormSearch.h"
#include "Dialogs.h"
#include "Liason.h"
#include "QtLyXView.h"
#include "buffer.h"
//#include "lyxtext.h"
#include "lyxfind.h"
//#include "language.h"
#include "support/lstrings.h"

using Liason::setMinibuffer;

FormSearch::FormSearch(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
   // let the dialog be shown
   // This is a permanent connection so we won't bother
   // storing a copy because we won't be disconnecting.
   d->showSearch.connect(SigC::slot(this, &FormSearch::show));
   // perhaps in the future we'd like a
   // "search again" button/keybinding
//    d->searchAgain.connect(slot(this, &FormSearch::FindNext));
}


FormSearch::~FormSearch()
{
   delete dialog_;
}

void FormSearch::show()
{
   if (!dialog_) {
      dialog_ = new SearchDlgImpl(this, 0, _("Find and Replace"), false);
   }

   if (!dialog_->isVisible()) {
      h_ = d_->hideBufferDependent.connect(SigC::slot(this, &FormSearch::hide));
      u_ = d_->updateBufferDependent.connect(SigC::slot(this, &FormSearch::update));
   }
   
   dialog_->raise();
   dialog_->setActiveWindow();
   update();
   dialog_->show();
}

void FormSearch::find(string const & searchstr, bool const & casesensitive,
		      bool const & matchword, bool const & searchback)
{
   bool found = LyXFind(lv_->view(), searchstr,	casesensitive, matchword,
			searchback);
   
   if (!found)
     setMinibuffer(lv_, _("String not found!"));
   
}
void FormSearch::replace(string const & searchstr, string const & replacestr,
			 bool const & casesensitive, bool const & matchword, 
			 bool const & searchback, bool const & replaceall)
{
   int replace_count = LyXReplace(lv_->view(), searchstr, replacestr,
				  casesensitive, matchword, searchback,
				  replaceall);
				  
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
}


void FormSearch::close()
{
   h_.disconnect();
   u_.disconnect();
}

void FormSearch::hide()
{
   dialog_->hide();
   close();
}

void FormSearch::update(bool)
{
   if (!lv_->view()->available())
     return;

   dialog_->setReadOnly(lv_->buffer()->isReadonly());
}
