/**
 * \file QSplash.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include <qframe.h>

#include "QSplashDialog.h"
#include "QSplash.h"
#include "Dialogs.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h"

QSplash::QSplash(LyXView *, Dialogs * d)
	: dialog_(0), d_(d)
{
	c_ = d->showSplash.connect(SigC::slot(this, &QSplash::show));
}


QSplash::~QSplash()
{
	c_.disconnect();
	delete dialog_;
}

 
void QSplash::hide()
{
	dialog_->hide();
}

 
void QSplash::show()
{
	if (!lyxrc.show_banner)
		return;

	if (!dialog_)
		dialog_ = new QSplashDialog( 0, "LyX");

	// show banner
	dialog_->show();
}
