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

#include <QCloseEvent>

#include "checkedwidgets.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "validators.h"

#include "controllers/ControlBibtex.h"
#include "controllers/ButtonPolicies.h"

#include "support/filetools.h"
#include "support/lstrings.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <q3listbox.h>

using lyx::support::changeExtension;
using lyx::support::trim;

using std::string;

namespace lyx {
namespace frontend {


QBibtexDialog::QBibtexDialog(QBibtex * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( stylePB, SIGNAL( clicked() ), this, SLOT( browsePressed() ) );
    connect( deletePB, SIGNAL( clicked() ), this, SLOT( deletePressed() ) );
    connect( styleCB, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( databaseLB, SIGNAL( selectionChanged() ), this, SLOT( databaseChanged() ) );
    connect( bibtocCB, SIGNAL( toggled(bool) ), this, SLOT( change_adaptor() ) );
    connect( btPrintCO, SIGNAL( activated(int) ), this, SLOT( change_adaptor() ) );
    connect( addBibPB, SIGNAL( clicked() ), this, SLOT( addPressed() ) );

	add_ = new UiDialog<Ui::QBibtexAddUi>(this, true);

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
	connect(add_->bibLB, SIGNAL(selected(Q3ListBoxItem *)),
		this, SLOT(addDatabase()));
	connect(add_->bibLB, SIGNAL(selected(Q3ListBoxItem *)),
		add_, SLOT(accept()));
	connect(add_->bibLB, SIGNAL(currentChanged(Q3ListBoxItem *)),
		this, SLOT(availableChanged()));
	connect(add_->browsePB, SIGNAL(clicked()),
		this, SLOT(browseBibPressed()));

    connect( add_->addPB, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( add_->closePB, SIGNAL( clicked() ), this, SLOT( reject() ) );

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
		string const filen = changeExtension(file, "");
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
		string const f = changeExtension(file, "");
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
	string const file = trim(fromqstr(add_->bibED->text()));

	if (sel < 0 && file.empty())
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

	if (!file.empty()) {
		QString const f = toqstr(changeExtension(file, ""));
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
