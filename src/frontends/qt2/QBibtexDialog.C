/**
 * \file QBibtexDialog.C
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

#include "ControlBibtex.h"
#include "gettext.h"
#include "debug.h"

#include "support/filetools.h"
#include "support/lyxalgo.h" // eliminate_duplicates

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>

#include "QBibtexDialog.h"
#include "QBibtex.h"

#include <vector>


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
	QString file =
		QFileDialog::getOpenFileName(QString::null,
					     _("BibTeX style files (*.bst)"),
					     this,
					     0,
					     _("Select a BibTeX style"));
	if (!file.isNull()) {
		string const filen = ChangeExtension(OnlyFilename(file.latin1()), "");
		styleCB->insertItem(filen.c_str(),0);
		form_->changed();
	}
}

void QBibtexDialog::addPressed()
{
	QString file = QFileDialog::getOpenFileName(QString::null,
		_("BibTeX database files (*.bib)"), this, 0, _("Select a BibTeX database to add"));
	if (!file.isNull()) {
		string const f = ChangeExtension(file.latin1(), "");
		bool present = false;
		for(unsigned int i = 0; i!=databaseLB->count(); i++) {
			if (databaseLB->text(i).latin1()==f)
				present = true;

		}
		if (!present) {
			databaseLB->insertItem(f.c_str());
			form_->changed();
		}
	}
}


void QBibtexDialog::deletePressed()
{
	databaseLB->removeItem(databaseLB->currentItem());
}


void QBibtexDialog::styleChanged(QString const & sel)
{
	styleCB->insertItem(sel,0);
}


void QBibtexDialog::databaseChanged()
{
	deletePB->setEnabled(!form_->readOnly() && databaseLB->currentItem() != -1);
}


void QBibtexDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}
