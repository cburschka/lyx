/**
 * \file QIndexDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"

#include "QIndex.h"
#include "QIndexDialog.h"
 
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qwhatsthis.h>

QIndexDialog::QIndexDialog(QIndex * form)
	: QIndexDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	QWhatsThis::add(keywordED, _(
"The format of the entry in the index.\n"
"\n"
"An entry can be specified as a sub-entry of\n"
"another with \"!\" :\n"
"\n"
"cars!mileage\n"
"\n"
"You can cross-refer to another entry like so :\n"
"\n"
"cars!mileage|see{economy}\n"
"\n"
"For further details refer to the local LaTeX\n"
"documentation.\n"));
}


void QIndexDialog::show()
{
	QIndexDialogBase::show(); 
	keywordED->setFocus();
}

 
void QIndexDialog::change_adaptor()
{
	form_->changed();
}


void QIndexDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
