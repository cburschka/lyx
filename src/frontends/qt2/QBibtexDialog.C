/**
 * \file QBibtexDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"

#include "support/filetools.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qfiledialog.h>

#include "ui/QBibtexAddDialogBase.h"
#include "QBibtexDialog.h"
#include "QBibtex.h"

using lyx::support::ChangeExtension;

using std::string;

namespace lyx {
namespace frontend {

QBibtexDialog::QBibtexDialog(QBibtex * form)
	: QBibtexDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	add_ = new QBibtexAddDialogBase(this, "", true);
	connect(add_->addPB, SIGNAL(clicked()), this, SLOT(addDatabase()));
	connect(add_->addPB, SIGNAL(clicked()), this, SLOT(addDatabase()));
	connect(add_->bibLB, SIGNAL(selected(QListBoxItem *)), this, SLOT(addDatabase()));
	connect(add_->bibLB, SIGNAL(selected(QListBoxItem *)), add_, SLOT(accept()));
	connect(add_->bibLB, SIGNAL(currentChanged(QListBoxItem *)), this, SLOT(availableChanged()));
	connect(add_->browsePB, SIGNAL(clicked()), this, SLOT(browseBibPressed()));
}


void QBibtexDialog::change_adaptor()
{
	form_->changed();
}


void QBibtexDialog::browsePressed()
{
	QString const file =
		QFileDialog::getOpenFileName(QString::null,
					     qt_("BibTeX style files (*.bst)"),
					     this,
					     0,
					     qt_("Select a BibTeX style"));
	if (!file.isNull()) {
		string const filen = ChangeExtension(fromqstr(file), "");
		bool present = false;
		unsigned int pres = 0;

		for (int i = 0; i != styleCB->count(); ++i) {
			if (fromqstr(styleCB->text(i)) == filen) {
				present = true;
				pres = i;
			}
		}

		if (!present)
			styleCB->insertItem(toqstr(filen),0);

		styleCB->setCurrentItem(pres);
		form_->changed();
	}
}


void QBibtexDialog::browseBibPressed()
{
	QString const file = QFileDialog::getOpenFileName(QString::null,
		qt_("BibTeX database files (*.bib)"), add_, 0, qt_("Select a BibTeX database to add"));

	if (!file.isNull()) {
		string const f = ChangeExtension(fromqstr(file), "");
		bool present = false;

		for (unsigned int i = 0; i != add_->bibLB->count(); i++) {
			if (fromqstr(add_->bibLB->text(i)) == f)
				present = true;
		}

		if (!present) {
			add_->bibLB->insertItem(toqstr(f));
			form_->changed();
		}

		add_->bibED->setText(toqstr(f));
	}
}

void QBibtexDialog::addPressed()
{
	add_->exec();
}


void QBibtexDialog::addDatabase()
{
	int const sel = add_->bibLB->currentItem();
	QString const file = add_->bibED->text();

	if (sel < 0 && file.isNull())
		return;

	// Add the selected browser_bib keys to browser_database
	// multiple selections are possible
	for (unsigned int i = 0; i != add_->bibLB->count(); i++) {
		if (add_->bibLB->isSelected(i)) {
			// do not allow duplicates
			if ((databaseLB->findItem(add_->bibLB->text(i))) == 0)
				databaseLB->insertItem(add_->bibLB->text(i));
		}
	}

	if (!file.isEmpty()) {
		QString const f = toqstr(ChangeExtension(fromqstr(file), ""));
		if ((databaseLB->findItem(f)) == 0)
			databaseLB->insertItem(f);
	}

	form_->changed();
}


void QBibtexDialog::deletePressed()
{
	databaseLB->removeItem(databaseLB->currentItem());
}



void QBibtexDialog::databaseChanged()
{
	deletePB->setEnabled(!form_->readOnly() && databaseLB->currentItem() != -1);
}


void QBibtexDialog::availableChanged()
{
	form_->changed();
}


void QBibtexDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
