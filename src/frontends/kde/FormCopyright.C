/**
 * \file FormCopyright.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 */
#include <config.h>

#include "Dialogs.h"
#include "FormCopyright.h"
#include "copyrightdlg.h"
#include "gettext.h"

FormCopyright::FormCopyright(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}


FormCopyright::~FormCopyright()
{
	delete dialog_;
}


void FormCopyright::show()
{
	if (!dialog_)
		dialog_ = new CopyrightDialog(0, _("LyX: Copyright and Warranty"));

	if (!dialog_->isVisible())
		h_ = d_->hideAll.connect(slot(this, &FormCopyright::hide));
 
	dialog_->show();
}


void FormCopyright::hide()
{
	dialog_->hide();
	h_.disconnect();
}
