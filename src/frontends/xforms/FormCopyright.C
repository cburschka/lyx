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
#include "form_copyright.h"
#include "FormCopyright.h"

FormCopyright::FormCopyright( LyXView * lv, Dialogs * d	)
	: FormBaseBI( lv, d, _("Copyright and Warranty") ),
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


FL_FORM * FormCopyright::form() const
{
	if ( dialog_ ) return dialog_->form;
	return 0;
}


void FormCopyright::build()
{
	dialog_ = build_copyright();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;
}
