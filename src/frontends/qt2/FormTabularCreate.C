/**
 * \file FormTabularCreate.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include "tabularcreatedlgimpl.h"
#include "Dialogs.h"
#include "FormTabularCreate.h"
#include "gettext.h"
#include "QtLyXView.h"
#include "BufferView.h"
#include "insets/insettabular.h"
#include "support/lstrings.h"

using SigC::slot;

FormTabularCreate::FormTabularCreate(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTabularCreate.connect(slot(this, &FormTabularCreate::show));
}

FormTabularCreate::~FormTabularCreate()
{
   delete dialog_;
}

void FormTabularCreate::apply(int rows, int cols)
{
   if (!lv_->view()->available())
     return;

   string tmp = tostr(rows) + " " + tostr(cols);
   lv_->getLyXFunc()->Dispatch(LFUN_INSET_TABULAR, tmp);
}

void FormTabularCreate::show()
{
   if (!dialog_)
     dialog_ = new TabularCreateDlgImpl(this, 0, _("LyX: Insert Table"));

   if (!dialog_->isVisible()) {
      h_ = d_->hideBufferDependent.connect(slot(this, &FormTabularCreate::hide));
      }

   dialog_->raise();
   dialog_->setActiveWindow();

   update();
   dialog_->show();
}

void FormTabularCreate::close()
{
   h_.disconnect();
}

void FormTabularCreate::hide()
{
   dialog_->hide();
   close();
}
