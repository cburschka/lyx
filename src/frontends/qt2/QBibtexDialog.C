/**
 * \file QBibtexDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
 
#include "QBibtexDialog.h"
#include "QBibtex.h"
#include "Dialogs.h"
#include "ControlBibtex.h" 
#include "gettext.h" 
#include "debug.h"

#include "support/filetools.h"

QBibtexDialog::QBibtexDialog(QBibtex * form)
	: QBibtexDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QBibtexDialog::change_adaptor()
{
	form_->changed();
}


void QBibtexDialog::browsePressed()
{
	QString file = QFileDialog::getOpenFileName(QString::null, 
		_("BibTeX style files (*.bst)"), this, 0, _("Select a BibTeX style"));
	lyxerr << "HELLO" << std::endl;
	if (!file.isNull()) {
		lyxerr << "MONKEYS" << std::endl;
		styleED->setText(ChangeExtension(OnlyFilename(file.latin1()), "").c_str());
		form_->changed();
	}
}

 
void QBibtexDialog::addPressed()
{
	string file(QFileDialog::getOpenFileName(QString::null, 
		_("BibTeX database files (*.bib)"), this, 0, _("Select a BibTeX database to add")).latin1()); 
	if (!file.empty()) {
		// FIXME: check duplicates
		databaseLB->insertItem(ChangeExtension(file, "").c_str());
		form_->changed();
	}
}

 
void QBibtexDialog::styleChanged(const QString & sel)
{
	if (string(_("Other ...")) == sel.latin1() && !form_->readOnly()) {
		styleED->setEnabled(true);
		stylePB->setEnabled(true);
	} else {
		styleED->setEnabled(false);
		stylePB->setEnabled(false);
		styleED->setText("");
	}
}

 
void QBibtexDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}
