/**********************************************************************

	--- Qt Architect generated file ---

	File: copyrightdlg.h
	Last generated: Thu Oct 12 18:32:40 2000

 *********************************************************************/

#ifndef CopyrightDialog_included
#define CopyrightDialog_included

#include "copyrightdlgdata.h"

class CopyrightDialog : public CopyrightDialogData
{
    Q_OBJECT

public:

    CopyrightDialog
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~CopyrightDialog();

protected slots:

    virtual void clickedOK();
};
#endif // CopyrightDialog_included
