/**
 * \file copyrightdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include "copyrightdlg.h"

CopyrightDialog::CopyrightDialog(QWidget * parent, char const * name)
	: CopyrightDialogData(parent, name)
{
	setCaption(name);
}


CopyrightDialog::~CopyrightDialog()
{
}


void CopyrightDialog::clickedOK()
{
	hide();
}
