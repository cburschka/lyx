#ifndef FORMCOPYRIGHTDIALOG_H
#define FORMCOPYRIGHTDIALOG_H
#include "FormCopyrightDialogBase.h"

class FormCopyrightDialog : public FormCopyrightDialogBase
{ 
    Q_OBJECT

public:
    FormCopyrightDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FormCopyrightDialog();

};

#endif // FORMCOPYRIGHTDIALOG_H
