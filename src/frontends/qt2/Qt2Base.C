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
#include "Qt2Base.h"
#include "qt2BC.h"
#include "support/LAssert.h"

#include <stdio.h>


Qt2Base::Qt2Base(::ControlButton & c, QString const & t)
	: ViewBC<qt2BC>(c), title_(t)
{}



void Qt2Base::show()
{
    if (!form()) {
	build();
    }

    update();  // make sure its up-to-date

    if (form()->isVisible()) {
	form()->raise();
    } else {
	form()->setCaption( title_ );
	form()->show();
    }
}


void Qt2Base::hide()
{
    if (form() && form()->isVisible() )
	form()->hide();
}


// PENDING(kalle) Handle this with QValidator?
// void Qt2Base::InputCB(FL_OBJECT * ob, long data)
// {
// 	bc().input(input(ob, data));
// }


ButtonPolicy::SMInput Qt2Base::input(QWidget*, long)
{
    return ButtonPolicy::SMI_VALID;
}



void Qt2Base::slotWMHide()
{
    CancelButton();
}



void Qt2Base::slotApply()
{
    ApplyButton();
}


void Qt2Base::slotOK()
{
    OKButton();
}


void Qt2Base::slotCancel()
{
    CancelButton();
}


void Qt2Base::slotRestore()
{
    RestoreButton();
}


// PENDING(kalle) How to handle this?
// extern "C" void C_Qt2BaseInputCB(FL_OBJECT * ob, long d)
// {
// 	GetForm(ob)->InputCB(ob, d);
// }
