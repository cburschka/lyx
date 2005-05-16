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

#include "QBibtexDialog.h"
#include "ui/QBibtexAddDialogBase.h"
#include "QBibtex.h"

#include "checkedwidgets.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "validators.h"

#include "controllers/ControlBibtex.h"
#include "controllers/ButtonPolicies.h"

#include "support/filetools.h"
#include "support/lstrings.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>

using lyx::support::ChangeExtension;
using lyx::support::trim;

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
	Qt2BC * bcview = new Qt2BC(add_bc_);
	add_bc_.view(bcview);
	add_bc_.bp(new OkCancelPolicy);

	bcview->setOK(add_->addPB);
	bcview->setCancel(add_->closePB);

	add_->bibED->setValidator(new PathValidator(false, add_->bibED));
	addCheckedLineEdit(add_bc_.view(), add_->bibED, 0);

	connect(add_->bibED, SIGNAL(textChanged(const QString&)),
		this, SLOT(bibEDChanged()));
	connect(add_->addPB, SIGNAL(clicked()),
		this, SLOT(addDatabase()));
	connect(add_->bibLB, SIGNAL(selected(QListBoxItem *)),
		this, SLOT(addDatabase()));
	connect(add_->bibLB, SIGNAL(selected(QListBoxItem *)),
		add_, SLOT(accept()));
	connect(add_->bibLB, SIGNAL(currentChanged(QListBoxItem *)),
		this, SLOT(availableChanged()));
	connect(add_->browsePB, SIGNAL(clicked()),
		this, SLOT(browseBibPressed()));
}


QBibtexDialog::~QBibtexDialog()
{}


void QBibtexDialog::bibEDChanged()
{
	// Indicate to the button controller that the contents have
	// changed. The actual test of validity is carried out by
	// the checkedLineEdit.
	add_bc_.valid(true);
}


void QBibtexDialog::change_adaptor()
{
	form_->changed();
}


void QBibtexDialog::browsePressed()
{
	string const file = form_->controller().browseBst("");

	if (!file.empty()) {
		string const filen = ChangeExtension(file, "");
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
	string const file = trim(form_->controller().browseBib(""));

	if (!file.empty()) {
		string const f = ChangeExtension(file, "");
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
	add_bc_.valid(false);
}


void QBibtexDialog::addDatabase()
{
	int const sel = add_->bibLB->currentItem();
	QString const file = trim(add_->bibED->text());

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
