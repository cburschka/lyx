/**
 * $Id: FormCitationDialogImpl.h,v 1.1 2001/03/16 17:20:06 kalle Exp $
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
    virtual void slotInsetSelected( int sel );
    virtual void slotAddClicked();
    virtual void slotDelClicked();
    virtual void slotUpClicked();
    virtual void slotDownClicked();
    virtual void apply_adaptor();
    virtual void close_adaptor();

private:
    FormCitation* form_;
};

#endif // FORMCITATIONDIALOGIMPL_H
