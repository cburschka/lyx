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

#include <stdio.h>


FormBase::FormBase(ControlBase & c, string const & t)
	: ViewBC<qt2BC>(c), title_(t)
{}



void FormBase::show()
{
    fprintf( stderr, "FormBase::show() 1\n" );
    if (!form()) {
    fprintf( stderr, "FormBase::show() 2\n" );
		build();
    fprintf( stderr, "FormBase::show() 3\n" );
	}
    fprintf( stderr, "FormBase::show() 4\n" );

	update();  // make sure its up-to-date
    fprintf( stderr, "FormBase::show() 5\n" );

	if (form()->isVisible()) {
    fprintf( stderr, "FormBase::show() 6\n" );
	    form()->raise();
    fprintf( stderr, "FormBase::show() 7\n" );
	} else {
    fprintf( stderr, "FormBase::show() 8\n" );
	    form()->setCaption( title_.c_str() );
    fprintf( stderr, "FormBase::show() 9\n" );
	    form()->show();
    fprintf( stderr, "FormBase::show() 10\n" );
	}
}


void FormBase::hide()
{
    if (form() && form()->isVisible() )
	form()->hide();
}


// PENDING(kalle) Handle this with QValidator?
// void FormBase::InputCB(FL_OBJECT * ob, long data)
// {
// 	bc().input(input(ob, data));
// }


ButtonPolicy::SMInput FormBase::input(QWidget*, long)
{
    return ButtonPolicy::SMI_VALID;
}



void FormBase::slotWMHide()
{
    CancelButton();
}



void FormBase::slotApply()
{
    ApplyButton();
}


void FormBase::slotOK()
{
    OKButton();
}


void FormBase::slotCancel()
{
    CancelButton();
}


void FormBase::slotRestore()
{
    RestoreButton();
}


// PENDING(kalle) How to handle this?
// extern "C" void C_FormBaseInputCB(FL_OBJECT * ob, long d)
// {
// 	GetForm(ob)->InputCB(ob, d);
// }
