#include "FormCopyrightDialog.h"

/* 
 *  Constructs a FormCopyrightDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormCopyrightDialog::FormCopyrightDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : FormCopyrightDialogBase( parent, name, modal, fl )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormCopyrightDialog::~FormCopyrightDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

