/**********************************************************************

	--- Qt Architect generated file ---

	File: paraextradialog.h
	Last generated: Sat Oct 14 00:27:49 2000

 *********************************************************************/

#ifndef ParaExtraDialog_included
#define ParaExtraDialog_included

#include "paraextradlgdata.h"

class ParaDialog;

class ParaExtraDialog : public ParaExtraDialogData
{
    Q_OBJECT

public:

    ParaExtraDialog
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~ParaExtraDialog();

    friend class ParaDialog;

protected slots:

    void typeHighlighted(int);

};
#endif // ParaExtraDialog_included
