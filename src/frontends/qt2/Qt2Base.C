/**
 * \file Qt2Base.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <qdialog.h>
#include <qapplication.h>

#include "debug.h"
#include "QtLyXView.h"
#include "Qt2Base.h"
#include "Qt2BC.h"
#include "ButtonController.h"
#include "ControlButtons.h"
#include "support/LAssert.h"


Qt2Base::Qt2Base(QString const & t)
	: ViewBase(), updating_(false), title_(t)
{}


Qt2BC & Qt2Base::bcview()
{
	return static_cast<Qt2BC &>(bc().view());
	// return dynamic_cast<Qt2BC &>(bc());
}


bool Qt2Base::isVisible() const
{
	return form() && form()->isVisible();
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
	bc().valid(isValid());
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
