/***************************************************************************
                          formcopyrightdialog.C  -  description
                             -------------------
    begin                : Fri Feb 4 2000
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
#include "formcopyrightdialog.h"

FormCopyrightDialog::FormCopyrightDialog(QWidget *parent, const char *name,
					 bool, WFlags)
	: QDialog(parent,name,false)
{
	initDialog();
	labelWho->setAlignment(AlignHCenter | AlignVCenter);
	labelWho->setFrameStyle(QFrame::Box | QFrame::Sunken);
	labelGNU->setAlignment(AlignHCenter | AlignVCenter);
	labelGNU->setFrameStyle(QFrame::Box | QFrame::Sunken);
	labelLyX->setAlignment(AlignHCenter | AlignVCenter);
	labelLyX->setFrameStyle(QFrame::Box | QFrame::Sunken);
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(reject()));
}

FormCopyrightDialog::~FormCopyrightDialog()
{
}
