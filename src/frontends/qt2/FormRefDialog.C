#include "FormRefDialog.h"
#include "Dialogs.h"
#include "FormRef.h"

/* 
 *  Constructs a FormRefDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormRefDialog::FormRefDialog( FormRef* _form, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormReferenceDialogBase( parent, name, modal, fl ),
    form( _form )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormRefDialog::~FormRefDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void FormRefDialog::apply_adaptor()
{
  form->apply();
  form->close();
  hide();
}
/* 
 * public slot
 */
void FormRefDialog::goto_adaptor()
{
  form->goto_ref();
}
/* 
 * public slot
 */
void FormRefDialog::highlight_adaptor(const QString& sel)
{
  form->highlight(sel); 
}
/* 
 * public slot
 */
void FormRefDialog::close_adaptor()
{
  form->close();
  hide();
}
/* 
 * public slot
 */
void FormRefDialog::select_adaptor(const QString& sel)
{
  form->select(sel);
}
/* 
 * public slot
 */
void FormRefDialog::sort_adaptor(bool sort)
{
  form->set_sort(sort);
}
/* 
 * public slot
 */
void FormRefDialog::update_adaptor()
{
  form->do_ref_update();
}

void FormRefDialog::closeEvent(QCloseEvent *e)
{
  form->close();
  e->accept();
}
