/**
 * \file copyrightdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
 
#include "FormCopyright.h" 
#include "copyrightdlg.h"

CopyrightDialog::CopyrightDialog(FormCopyright * f, QWidget * parent, char const * name)
	: CopyrightDialogData(parent, name), form_(f)
{
	setCaption(name);
}


CopyrightDialog::~CopyrightDialog()
{
}


void CopyrightDialog::clickedOK()
{
	form_->OKButton();
}
