#include "FormCreditsDialogImpl.h"
#include "FormCredits.h"
#include <qpushbutton.h>

/*
 *  Constructs a FormCreditsDialogImpl which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FormCreditsDialogImpl::FormCreditsDialogImpl( FormCredits* form, QWidget* parent,  
					      const char* name, bool modal, WFlags fl )
    : FormCreditsDialog( parent, name, modal, fl )
{
    connect( okPB, SIGNAL( clicked() ),
	     form, SLOT( slotCancel() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
FormCreditsDialogImpl::~FormCreditsDialogImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

