#include "FormIndexDialog.h"
#include "Dialogs.h"
#include "FormIndex.h"

/* 
 *  Constructs a FormIndexDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormIndexDialog::FormIndexDialog( FormIndex* form_, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormIndexDialogBase( parent, name, modal, fl ),
    form( form_ )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormIndexDialog::~FormIndexDialog()
{
    // no need to delete child widgets, Qt does it all for us
}


void FormIndexDialog::apply_adaptor()
{
  form->apply();
  form->close();
  hide();
}


void FormIndexDialog::close_adaptor()
{
  form->close();
  hide();
}


void FormIndexDialog::closeEvent(QCloseEvent *e)
{
  form->close();
  e->accept();
}
