#ifndef FORMCOPYRIGHTDIALOGIMPL_H
#define FORMCOPYRIGHTDIALOGIMPL_H
#include "FormCopyrightDialog.h"

class FormCopyrightDialogImpl : public FormCopyrightDialog
{ 
    Q_OBJECT

public:
    FormCopyrightDialogImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormCopyrightDialogImpl();

};

#endif // FORMCOPYRIGHTDIALOGIMPL_H
