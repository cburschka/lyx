#include "FormUrlDialog.h"
#include "Dialogs.h"
#include "FormUrl.h"

/* 
 *  Constructs a FormUrlDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormUrlDialog::FormUrlDialog( FormUrl* _form, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormUrlDialogBase( parent, name, modal, fl ),
    form( _form )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormUrlDialog::~FormUrlDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void FormUrlDialog::apply_adaptor()
{
  form->apply();
  form->close();
  hide();
}
/* 
 * public slot
 */
void FormUrlDialog::close_adaptor()
{
  form->close();
  hide();
}

void FormUrlDialog::closeEvent(QCloseEvent *e)
{
  form->close();
  e->accept();
}
