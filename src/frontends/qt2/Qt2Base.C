/* This file is part of
 * ======================================================
 *
 *		   LyX, The Document Processor
 *
 *		   Copyright 2000 The LyX Team.
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
#include <qapplication.h> 

#include "debug.h" 
#include "QtLyXView.h" 
#include "Dialogs.h"
#include "Qt2Base.h"
#include "Qt2BC.h"
#include "support/LAssert.h"

Qt2Base::Qt2Base(ControlButtons & c, QString const & t)
	: ViewBC<Qt2BC>(c), title_(t)
{}


void Qt2Base::show()
{
	if (!form()) {
		build();
	}

	form()->setMinimumSize(form()->sizeHint());

	update();  // make sure its up-to-date

	if (form()->isVisible()) {
		form()->raise();
	} else {
		form()->setCaption(title_);
		form()->show();
	}
}


void Qt2Base::reset()
{
	qApp->processEvents();
}

 
void Qt2Base::hide()
{
	if (form() && form()->isVisible())
		form()->hide();
}


bool Qt2Base::isValid()
{
	return true;
}

 
void Qt2Base::changed()
{
	if (isValid())
		bc().valid(); 
	else
		bc().invalid();
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


void Qt2Base::slotClose()
{
	CancelButton();
}


void Qt2Base::slotRestore()
{
	RestoreButton();
}
