/**********************************************************************

	--- Qt Architect generated file ---

	File: formcopyrightdialog.C
	Last generated: Thu Oct 12 18:32:40 2000

 *********************************************************************/

#include "formcopyrightdialog.h"

#define Inherited FormCopyrightDialogData

FormCopyrightDialog::FormCopyrightDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
	setCaption(name);
}


FormCopyrightDialog::~FormCopyrightDialog()
{
}

void FormCopyrightDialog::clickedOK()
{
	hide();
}
