/**********************************************************************

	--- Qt Architect generated file ---

	File: parageneraldlg.h
	Last generated: Sat Oct 14 00:27:47 2000

 *********************************************************************/

#ifndef ParaGeneralDialog_included
#define ParaGeneralDialog_included

#include "parageneraldlgdata.h"

class ParaDialog;

class ParaGeneralDialog : public ParaGeneralDialogData
{
    Q_OBJECT

public:

    ParaGeneralDialog
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~ParaGeneralDialog();

    friend class ParaDialog;

protected slots:

    void spaceaboveHighlighted(int);
    void spacebelowHighlighted(int);
    void spaceabovevalueChanged(const char *text);
    void spaceaboveplusChanged(const char *text);
    void spaceaboveminusChanged(const char *text);
    void spacebelowvalueChanged(const char *text);
    void spacebelowplusChanged(const char *text);
    void spacebelowminusChanged(const char *text);

private:

	void createUnits(QComboBox *box);
};
#endif // ParaGeneralDialog_included
