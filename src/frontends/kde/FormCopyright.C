/***************************************************************************
                          formcopyright.cpp  -  description
                             -------------------
    begin                : Thu Feb 3 2000
    copyright            : (C) 2000 by Jürgen Vigna
    email                : jug@sad.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include "Dialogs.h"
#include "FormCopyright.h"
#include "formcopyrightdialog.h"
#include "gettext.h"

FormCopyright::FormCopyright(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}

FormCopyright::~FormCopyright()
{
	delete dialog_;
}

void FormCopyright::show()
{
	if (!dialog_)
		dialog_ = new FormCopyrightDialog(0, _("Copyright and Warranty"),
						  false);
	if (!dialog_->isVisible())
		h_ = d_->hideAll.connect(slot(this, &FormCopyright::hide));
	dialog_->show();
}

void FormCopyright::hide()
{
	dialog_->hide();
	h_.disconnect();
}
