#include <config.h>
#include <debug.h>
#include "FormTocDialog.h"
#include "Dialogs.h"
#include "FormToc.h"

#include <qlistview.h>

/* 
 *  Constructs a FormTocDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormTocDialog::FormTocDialog( FormToc* form_, QWidget* parent,  const char* name, bool modal, WFlags fl )
  : FormTocDialogBase( parent, name, modal, fl ),
    form( form_ )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
FormTocDialog::~FormTocDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */
void FormTocDialog::activate_adaptor(int index)
{
  switch (index) {
  case 0:
    form->set_type(Buffer::TOC_TOC);
    break;
  case 1:
    form->set_type(Buffer::TOC_LOF);
    break;
  case 2:
    form->set_type(Buffer::TOC_LOT);
    break;
  case 3:
    form->set_type(Buffer::TOC_LOA);
    break;
  default:
    lyxerr[Debug::GUI] << "Unknown TOC combo selection." << std::endl;
    break;
  }
}
/* 
 * public slot
 */
void FormTocDialog::close_adaptor()
{
  form->close();
  hide();
}
/* 
 * public slot
 */
void FormTocDialog::depth_adaptor(int depth)
{
  form->set_depth(depth);
}
/* 
 * public slot
 */
void FormTocDialog::select_adaptor(QListViewItem* item)
{
  form->select(item->text(0));
}
/* 
 * public slot
 */
void FormTocDialog::update_adaptor()
{
  form->update();
}

void FormTocDialog::closeEvent(QCloseEvent *e)
{
  form->close();
  e->accept();
}
