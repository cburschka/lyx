/**********************************************************************

	--- Qt Architect generated file ---

	File: formindexdialog.h
	Last generated: Thu Sep 14 12:08:37 2000

 *********************************************************************/

#ifndef FormIndexDialog_included
#define FormIndexDialog_included

#include "formindexdialogdata.h"

class FormIndex;

class FormIndexDialog : public FormIndexDialogData
{
    Q_OBJECT

public:

    FormIndexDialog
    (
        FormIndex *, QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~FormIndexDialog();

    void setIndexText(const char * str) { index->setText(str); }
    const char * getIndexText() { return index->text(); }
    void setReadOnly(bool);

private slots:
    void clickedOK();
    void clickedCancel();

private:
    FormIndex *form_;

};
#endif // FormIndexDialog_included
