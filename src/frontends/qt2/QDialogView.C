/**
 * \file QDialogView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>



#include "qt_helpers.h"
#include "QDialogView.h"
#include "Qt2BC.h"
#include "ButtonController.h"


QDialogView::QDialogView(Dialog & parent, string const & t)
	: Dialog::View(parent,t), updating_(false)
{}


Qt2BC & QDialogView::bcview()
{
	return static_cast<Qt2BC &>(dialog().bc().view());
}


bool QDialogView::isVisible() const
{
	return form() && form()->isVisible();
}


bool QDialogView::readOnly() const
{
	return kernel().isBufferReadonly();
}


void QDialogView::show()
{
	if (!form()) {
		build();
	}

	form()->setMinimumSize(form()->sizeHint());

	update();  // make sure its up-to-date

	form()->setCaption(toqstr(getTitle()));

	if (form()->isVisible()) {
		form()->raise();
	} else {
		form()->show();
	}
}


void QDialogView::hide()
{
	if (form() && form()->isVisible())
		form()->hide();
}


bool QDialogView::isValid()
{
	return true;
}


void QDialogView::changed()
{
	if (updating_)
		return;
	bc().valid(isValid());
}


void QDialogView::slotWMHide()
{
	dialog().CancelButton();
}


void QDialogView::slotApply()
{
	dialog().ApplyButton();
}


void QDialogView::slotOK()
{
	dialog().OKButton();
}


void QDialogView::slotClose()
{
	dialog().CancelButton();
}


void QDialogView::slotRestore()
{
	dialog().RestoreButton();
}
