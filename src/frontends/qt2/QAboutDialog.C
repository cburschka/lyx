/**
 * \file QAboutDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#include "QAboutDialog.h"

QAboutDialog::QAboutDialog(QWidget * parent,  const char * name, bool modal, WFlags fl)
    : QAboutDialogBase(parent, name, modal, fl)
{
}


QAboutDialog::~QAboutDialog()
{
}
