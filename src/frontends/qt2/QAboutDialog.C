/**
 * \file QAboutDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QAboutDialog.h"

QAboutDialog::QAboutDialog(QWidget * parent,  const char * name, bool modal, WFlags fl)
    : QAboutDialogBase(parent, name, modal, fl)
{
}


QAboutDialog::~QAboutDialog()
{
}
