/**
 * \file QDialogView.C
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
#include "Dialogs.h"
#include "QDialogView.h"
#include "Qt2BC.h"
#include "support/LAssert.h"


QDialogView::QDialogView(Dialog & parent, QString const & t)
	: Dialog::View(parent), updating_(false), title_(t)
{}


Qt2BC & QDialogView::bc()
{
	return static_cast<Qt2BC &>(dialog().bc());
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

	if (form()->isVisible()) {
		form()->raise();
	} else {
		form()->setCaption(title_);
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

	if (isValid())
		bc().valid();
	else
		bc().invalid();
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
