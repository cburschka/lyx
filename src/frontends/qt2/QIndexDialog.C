/**
 * \file QIndexDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QIndexDialog.h"
#include "Dialogs.h"
#include "QIndex.h"
#include "gettext.h"

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


void QIndexDialog::change_adaptor()
{
	form_->changed();
}


void QIndexDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
