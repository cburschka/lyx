/**
 * $Id: FormCitationDialogImpl.h,v 1.3 2001/03/31 08:39:24 kalle Exp $
 */

#ifndef FORMCITATIONDIALOGIMPL_H
#define FORMCITATIONDIALOGIMPL_H
#include "FormCitationDialog.h"

class FormCitation;

class FormCitationDialogImpl : public FormCitationDialog
{ 
    Q_OBJECT

public:
    FormCitationDialogImpl( FormCitation* form, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormCitationDialogImpl();

protected slots:
    virtual void slotBibSelected( int sel );
    virtual void slotCiteSelected( int sel );
    virtual void slotAddClicked();
    virtual void slotDelClicked();
    virtual void slotUpClicked();
    virtual void slotDownClicked();
    virtual void slotPreviousClicked();
    virtual void slotNextClicked();
    virtual void slotCitationStyleSelected( int );
    virtual void slotTextBeforeReturn();
    virtual void slotTextAfterReturn();

private:
    void doPreviousNext( bool );
    
private:
    FormCitation* form_;
};

#endif // FORMCITATIONDIALOGIMPL_H
