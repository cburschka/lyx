/**********************************************************************

	--- Qt Architect generated file ---

	File: indexdlg.h
	Last generated: Thu Sep 14 12:08:37 2000

 *********************************************************************/

#ifndef IndexDialog_included
#define IndexDialog_included

#include "indexdlgdata.h"

class FormIndex;

class IndexDialog : public IndexDialogData
{
    Q_OBJECT

public:

    IndexDialog
    (
        FormIndex *, QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~IndexDialog();

    void setIndexText(const char * str) { index->setText(str); }
    const char * getIndexText() { return index->text(); }
    void setReadOnly(bool);

private slots:
    void clickedOK();
    void clickedCancel();

private:
    FormIndex *form_;

};
#endif // IndexDialog_included
