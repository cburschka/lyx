// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <qdialog.h>
#undef emit

#include "Dialogs.h"
#include "FormBase.h"
#include "qt2BC.h"
#include "support/LAssert.h"


FormBase::FormBase(ControlBase & c, string const & t)
	: ViewBC<qt2BC>(c), title_(t)
{}



void FormBase::show()
{
	if (!form()) {
		build();
	}

	update();  // make sure its up-to-date

	if (form()->isVisible()) {
	    form()->raise();
	} else {
	    form()->setCaption( title_.c_str() );
	    form()->show();
	}
}


void FormBase::hide()
{
    if (form() && form()->isVisible() )
	form()->hide();
}


// void FormBase::InputCB(FL_OBJECT * ob, long data)
// {
// 	bc().input(input(ob, data));
// }


// ButtonPolicy::SMInput FormBase::input(FL_OBJECT *, long)
// {
// 	return ButtonPolicy::SMI_VALID;
// }


namespace {

// FormBase * GetForm(QWidget * ob)
// {
// 	Assert(ob && ob->form && ob->form->u_vdata);
// 	FormBase * pre = static_cast<FormBase *>(ob->form->u_vdata);
// 	return pre;
// }

} // namespace anon


// extern "C" int C_FormBaseWMHideCB(FL_FORM * form, void *)
// {
// 	// Close the dialog cleanly, even if the WM is used to do so.
// 	Assert(form && form->u_vdata);
// 	FormBase * pre = static_cast<FormBase *>(form->u_vdata);
// 	pre->CancelButton();
// 	return FL_CANCEL;
// }


// extern "C" void C_FormBaseApplyCB(FL_OBJECT * ob, long)
// {
// 	GetForm(ob)->ApplyButton();
// }


// extern "C" void C_FormBaseOKCB(FL_OBJECT * ob, long)
// {
// 	GetForm(ob)->OKButton();
// }


// extern "C" void C_FormBaseCancelCB(FL_OBJECT * ob, long)
// {
// 	FormBase * form = GetForm(ob);
// 	form->CancelButton();
// }


// extern "C" void C_FormBaseRestoreCB(FL_OBJECT * ob, long)
// {
// 	GetForm(ob)->RestoreButton();
// }


// extern "C" void C_FormBaseInputCB(FL_OBJECT * ob, long d)
// {
// 	GetForm(ob)->InputCB(ob, d);
// }
