/* FormBrowser.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "FormBrowser.h"
#include "form_browser.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "buffer.h"

FormBrowser::FormBrowser(LyXView * lv, Dialogs * d, const string & name)
	: FormBaseBD(lv, d, name)
{}


void FormBrowser::build()
{
	dialog_.reset(build_browser());

	// Manage the close button
	bc().setCancel(dialog_->button_close);
	bc().refresh();
}


FL_FORM * FormBrowser::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}

void FormBrowser::update()
{}


bool FormBrowser::input(FL_OBJECT *, long)
{
	update();
	return true;
}
