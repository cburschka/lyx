/* FormCopyright.C
 * FormCopyright Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "xform_macros.h"
#include "form_copyright.h"
#include "FormCopyright.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "gettext.h"

C_RETURNCB(FormCopyright, WMHideCB)
C_GENERICCB(FormCopyright, OKCB)

FormCopyright::FormCopyright(LyXView * lv, Dialogs * d)
	: dialog_(0), lv_(lv), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}


FormCopyright::~FormCopyright()
{
	free();
}


void FormCopyright::build()
{
  dialog_ = build_copyright();
}


void FormCopyright::show()
{
	if (!dialog_) {
		build();
		fl_set_form_atclose(dialog_->form_copyright,
				    C_FormCopyrightWMHideCB, 0);
	}

	if (dialog_->form_copyright->visible) {
		fl_raise_form(dialog_->form_copyright);
	} else {
		fl_show_form(dialog_->form_copyright,
			     FL_PLACE_MOUSE,
			     FL_FULLBORDER,
			     _("Copyright and Warranty"));
		h_ = d_->hideAll.connect(slot(this, &FormCopyright::hide));
	}
}


void FormCopyright::hide()
{
	if (dialog_
	    && dialog_->form_copyright
	    && dialog_->form_copyright->visible) {
		fl_hide_form(dialog_->form_copyright);
		h_.disconnect();
	}
	free();
}


void FormCopyright::free()
{
	// we don't need to delete h here because
	// hide() does that after disconnecting.
	if (dialog_) {
		if (dialog_->form_copyright
		    && dialog_->form_copyright->visible) {
			hide();
		}
		fl_free_form(dialog_->form_copyright);
		delete dialog_;
		dialog_ = 0;
	}
}


int FormCopyright::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signal h is disconnected even if the
	// window manager is used to close the dialog.
	FormCopyright * pre = static_cast<FormCopyright*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormCopyright::OKCB(FL_OBJECT * ob, long)
{
	FormCopyright * pre = static_cast<FormCopyright*>(ob->form->u_vdata);
	pre->hide();
}
