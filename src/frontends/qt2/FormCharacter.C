/**
 * \file FormCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#include "gettext.h"
#include "chardlgimpl.h"
#include "FormCharacter.h"
#include "bufferview_funcs.h"
#include "Dialogs.h"
#include "Liason.h"
#include "QtLyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "language.h"
#include "support/lstrings.h"

using SigC::slot;
using Liason::setMinibuffer;

FormCharacter::FormCharacter(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
   // let the dialog be shown
   // This is a permanent connection so we won't bother
   // storing a copy because we won't be disconnecting.
   d->showLayoutCharacter.connect(slot(this, &FormCharacter::show));
   // for LFUN_FREE
   d->setUserFreeFont.connect(slot(this, &FormCharacter::apply));
}


FormCharacter::~FormCharacter()
{
   delete dialog_;
}


void FormCharacter::apply()
{
   if (!lv_->view()->available() || !dialog_)
     return;

   LyXFont font = dialog_->getChar();

   if (dialog_->langItem()==1)
     font.setLanguage(lv_->buffer()->params.language);

   ToggleAndShow(lv_->view(), font, dialog_->toggleAll());
   lv_->view()->setState();
   lv_->buffer()->markDirty();
   setMinibuffer(lv_, _("Character set"));
}

void FormCharacter::show()
{
   if (!dialog_) {
      dialog_ = new CharDlgImpl(this, 0, _("Character Options"), false);
      // add languages
      for (Languages::const_iterator cit = languages.begin();
	   cit != languages.end(); ++cit) {
	 const string language = (*cit).second.lang();
	 dialog_->lang->insertItem( tostr(language).c_str(), -1 );
      }
   }

   if (!dialog_->isVisible()) {
      h_ = d_->hideBufferDependent.connect(slot(this, &FormCharacter::hide));
      u_ = d_->updateBufferDependent.connect(slot(this, &FormCharacter::update));
   }

   dialog_->raise();
   dialog_->setActiveWindow();
   update();
   dialog_->show();
}

void FormCharacter::close()
{
   h_.disconnect();
   u_.disconnect();
}

void FormCharacter::hide()
{
   dialog_->hide();
   close();
}

void FormCharacter::update(bool)
{

   if (!lv_->view()->available())
     return;

   dialog_->setReadOnly(lv_->buffer()->isReadonly());
}
