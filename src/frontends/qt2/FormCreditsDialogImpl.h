#ifndef FORMCREDITSDIALOGIMPL_H
#define FORMCREDITSDIALOGIMPL_H
#include "FormCreditsDialog.h"

class FormCredits;

class FormCreditsDialogImpl : public FormCreditsDialog
{ 
    Q_OBJECT

public:
    FormCreditsDialogImpl( FormCredits* form, QWidget* parent = 0, 
			   const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormCreditsDialogImpl();

};

#endif // FORMCREDITSDIALOGIMPL_H
