/**
 * \file indexdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>

#include "indexdlg.h"
#include "FormIndex.h"

#include <qtooltip.h>

IndexDialog::IndexDialog(FormIndex * f, QWidget * p, char const * name)
	: IndexDialogData(p, name), form_(f)
{
	setCaption(name);

	// tooltips

	QToolTip::add(labelindex,_("Index entry"));
	QToolTip::add(index,_("Index entry"));

	setMinimumSize(200, 65);
	setMaximumSize(32767, 65);
}


IndexDialog::~IndexDialog()
{
}


void IndexDialog::clickedOK()
{
	form_->apply();
	form_->close();
	hide();
}


void IndexDialog::clickedCancel()
{
	form_->close();
	hide();
}


void IndexDialog::setReadOnly(bool readonly)
{
	if (readonly) {
		index->setFocusPolicy(QWidget::NoFocus);
		buttonOk->setEnabled(false);
		buttonCancel->setText(_("&Close"));
	} else {
		index->setFocusPolicy(QWidget::StrongFocus);
		index->setFocus();
		buttonOk->setEnabled(true);
		buttonCancel->setText(_("&Cancel"));
	}
}
