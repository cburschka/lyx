/**********************************************************************

	--- Qt Architect generated file ---

	File: copyrightdlg.C
	Last generated: Thu Oct 12 18:32:40 2000

 *********************************************************************/

#include "copyrightdlg.h"

#define Inherited CopyrightDialogData

CopyrightDialog::CopyrightDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
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
