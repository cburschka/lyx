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
#include "ControlButtons.h"
#include "support/LAssert.h"

Qt2Base::Qt2Base(QString const & t)
	: ViewBase(), updating_(false), title_(t)
{}


Qt2BC & Qt2Base::bc()
{
	return static_cast<Qt2BC &>(getController().bc());
	// return dynamic_cast<Qt2BC &>(getController().bc());
}


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
	if (updating_)
		return;

	if (isValid())
		bc().valid();
	else
		bc().invalid();
}


void Qt2Base::slotWMHide()
{
	getController().CancelButton();
}


void Qt2Base::slotApply()
{
	getController().ApplyButton();
}


void Qt2Base::slotOK()
{
	getController().OKButton();
}


void Qt2Base::slotClose()
{
	getController().CancelButton();
}


void Qt2Base::slotRestore()
{
	getController().RestoreButton();
}
