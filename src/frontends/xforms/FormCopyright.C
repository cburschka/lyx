/* FormCopyright.C
 * FormCopyright Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "LyXView.h"
#include "gettext.h"
#include "form_copyright.h"
#include "FormCopyright.h"

FormCopyright::FormCopyright( LyXView * lv, Dialogs * d	)
	: FormBase( lv, d, BUFFER_INDEPENDENT, _("Copyright and Warranty") ),
	  dialog_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}


FormCopyright::~FormCopyright()
{
	delete dialog_;
}


void FormCopyright::build()
{
	dialog_ = build_copyright();
}


FL_FORM * const FormCopyright::form() const
{
	if( dialog_ ) // no need to test for dialog_->form
		return dialog_->form;
	else
		return 0;
}
