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

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

FormBrowser::FormBrowser(LyXView * lv, Dialogs * d, const string & name)
	: FormBaseBD(lv, d, name, new IgnorantPolicy),
	  dialog_(0)
{
}

FormBrowser::~FormBrowser()
{
	delete dialog_;
}

void FormBrowser::build()
{
	dialog_ = build_browser();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	// Manage the close button
	bc_.setOK(dialog_->button_close);
	bc_.refresh();
}

FL_FORM * FormBrowser::form() const
{
	if (dialog_)
		return dialog_->form;
	return 0;
}

void FormBrowser::update()
{
}

bool FormBrowser::input(FL_OBJECT *, long)
{
	update();
	return true;
}
