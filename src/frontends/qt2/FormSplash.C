/**
 * \file FormSplash.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include <qframe.h>

#include "FormSplashImpl.h"
#include "FormSplash.h"
#include "Dialogs.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h"

FormSplash::FormSplash(LyXView *, Dialogs * d)
	: dialog_(0), d_(d)
{
   c_ = d->showSplash.connect(SigC::slot(this, &FormSplash::show));
}


FormSplash::~FormSplash()
{
   c_.disconnect();
   delete dialog_;
}

void FormSplash::hide()
{
   dialog_->hide();
}

void FormSplash::show()
{
   if (!lyxrc.show_banner)
     return;

   if (!dialog_)
     dialog_ = new FormSplashBase( 0, "LyX");

   // show banner
   dialog_->show();
}
