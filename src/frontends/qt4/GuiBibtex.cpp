/**
 * \file GuiBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voß
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBibtex.h"

#include "ui_BibtexAddUi.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "Validator.h"
#include "LyXRC.h"
#include "CheckedLineEdit.h"

#include "controllers/ControlBibtex.h"
#include "controllers/ButtonPolicy.h"

#include "controllers/ControlBibtex.h"

#include "support/filetools.h" // changeExtension
#include "support/lstrings.h"

#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>

using lyx::support::changeExtension;
using lyx::support::split;
using lyx::support::trim;

using std::vector;
using std::string;

#include "debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"


namespace lyx {
namespace frontend {


/////////////////////////////////////////////////////////////////////
//
// GuiBibtexDialog
//
/////////////////////////////////////////////////////////////////////

GuiBibtexDialog::GuiBibtexDialog(GuiBibtex * form)
	: form_(form)
{
	setupUi(this);
	QDialog::setModal(true);

	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(stylePB, SIGNAL(clicked()),
		this, SLOT(browsePressed()));
	connect(deletePB, SIGNAL(clicked()),
		this, SLOT(deletePressed()));
	connect(styleCB, SIGNAL(editTextChanged (const QString &)),
		this, SLOT(change_adaptor()));
	connect(databaseLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(databaseChanged()));
	connect(bibtocCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(btPrintCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(addBibPB, SIGNAL(clicked()),
		this, SLOT(addPressed()));

	add_ = new UiDialog<Ui::BibtexAddUi>(this, true);

	Qt2BC * bcview = new Qt2BC(add_bc_);
	add_bc_.view(bcview);
	add_bc_.bp(new OkCancelPolicy);

	bcview->setOK(add_->addPB);
	bcview->setCancel(add_->closePB);

	addCheckedLineEdit(add_bc_.view(), add_->bibED, 0);

	connect(add_->bibED, SIGNAL(textChanged(const QString &)),
		this, SLOT(bibEDChanged()));
	connect(add_->addPB, SIGNAL(clicked()),
		this, SLOT(addDatabase()));
	connect(add_->addPB, SIGNAL(clicked()),
		add_, SLOT(accept()));
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(addDatabase()));
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		add_, SLOT(accept()));
	connect(add_->bibLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(availableChanged()));
	connect(add_->browsePB, SIGNAL(clicked()),
		this, SLOT(browseBibPressed()));
	connect(add_->closePB, SIGNAL(clicked()),
		add_, SLOT(reject()));

}


void GuiBibtexDialog::bibEDChanged()
{
	// Indicate to the button controller that the contents have
	// changed. The actual test of validity is carried out by
	// the checkedLineEdit.
	add_bc_.valid(true);
}


void GuiBibtexDialog::change_adaptor()
{
	form_->changed();
}


void GuiBibtexDialog::browsePressed()
{
	docstring const file = form_->controller().browseBst(docstring());

	if (!file.empty()) {
		// FIXME UNICODE
		docstring const filen = from_utf8(changeExtension(to_utf8(file), ""));
		bool present = false;
		unsigned int pres = 0;

		for (int i = 0; i != styleCB->count(); ++i) {
			if (qstring_to_ucs4(styleCB->itemText(i)) == filen) {
				present = true;
				pres = i;
			}
		}

		if (!present)
			styleCB->insertItem(0, toqstr(filen));

		styleCB->setCurrentIndex(pres);
		form_->changed();
	}
}


void GuiBibtexDialog::browseBibPressed()
{
	docstring const file = trim(form_->controller().browseBib(docstring()));

	if (!file.empty()) {
		// FIXME UNICODE
		QString const f = toqstr(changeExtension(to_utf8(file), ""));
		bool present = false;

		for (int i = 0; i < add_->bibLW->count(); ++i) {
			if (add_->bibLW->item(i)->text() == f)
				present = true;
		}

		if (!present) {
			add_->bibLW->addItem(f);
			form_->changed();
		}

		add_->bibED->setText(f);
	}
}


void GuiBibtexDialog::addPressed()
{
	add_bc_.valid(false);
	add_->exec();
}


void GuiBibtexDialog::addDatabase()
{
	int const sel = add_->bibLW->currentRow();
	docstring const file = trim(qstring_to_ucs4(add_->bibED->text()));

	if (sel < 0 && file.empty())
		return;

	// Add the selected browser_bib keys to browser_database
	// multiple selections are possible
	for (int i = 0; i != add_->bibLW->count(); ++i) {
		QListWidgetItem * const item = add_->bibLW->item(i);
		if (add_->bibLW->isItemSelected(item)) {
			add_->bibLW->setItemSelected(item, false);
			QList<QListWidgetItem *> matches =
				databaseLW->findItems(item->text(), Qt::MatchExactly);
			if (matches.empty())
				databaseLW->addItem(item->text());
		}
	}

	if (!file.empty()) {
		add_->bibED->clear();
		QString const f = toqstr(from_utf8(changeExtension(to_utf8(file), "")));
		QList<QListWidgetItem *> matches =
			databaseLW->findItems(f, Qt::MatchExactly);
		if (matches.empty())
			databaseLW->addItem(f);
	}

	form_->changed();
}


void GuiBibtexDialog::deletePressed()
{
	databaseLW->takeItem(databaseLW->currentRow());
	form_->changed();
}



void GuiBibtexDialog::databaseChanged()
{
	deletePB->setEnabled(!form_->readOnly() && databaseLW->currentRow() != -1);
}


void GuiBibtexDialog::availableChanged()
{
	add_bc_.valid(true);
}


void GuiBibtexDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// QBibTex
//
/////////////////////////////////////////////////////////////////////


typedef QController<ControlBibtex, GuiView<GuiBibtexDialog> > BibtexBase;

GuiBibtex::GuiBibtex(Dialog & parent)
	: BibtexBase(parent, _("BibTeX Bibliography"))
{
}


void GuiBibtex::build_dialog()
{
	dialog_.reset(new GuiBibtexDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->databaseLW);
	bcview().addReadOnly(dialog_->stylePB);
	bcview().addReadOnly(dialog_->styleCB);
	bcview().addReadOnly(dialog_->bibtocCB);
	bcview().addReadOnly(dialog_->addBibPB);
	bcview().addReadOnly(dialog_->deletePB);
}


void GuiBibtex::update_contents()
{
	bool bibtopic = controller().usingBibtopic();

	dialog_->databaseLW->clear();

	docstring bibs(controller().params()["bibfiles"]);
	docstring bib;

	while (!bibs.empty()) {
		bibs = split(bibs, bib, ',');
		bib = trim(bib);
		if (!bib.empty())
			dialog_->databaseLW->addItem(toqstr(bib));
	}

	dialog_->add_->bibLW->clear();

	vector<string> bib_str;
	controller().getBibFiles(bib_str);
	for (vector<string>::const_iterator it = bib_str.begin();
		it != bib_str.end(); ++it) {
		string bibItem(changeExtension(*it, ""));
		dialog_->add_->bibLW->addItem(toqstr(bibItem));
	}

	string bibstyle(controller().getStylefile());

	dialog_->bibtocCB->setChecked(controller().bibtotoc() && !bibtopic);
	dialog_->bibtocCB->setEnabled(!bibtopic);

	docstring btprint(controller().params()["btprint"]);
	int btp = 0;
	if (btprint == "btPrintNotCited")
		btp = 1;
	else if (btprint == "btPrintAll")
		btp = 2;

	dialog_->btPrintCO->setCurrentIndex(btp);
	dialog_->btPrintCO->setEnabled(bibtopic);

	dialog_->styleCB->clear();

	int item_nr(-1);

	vector<string> str;
	controller().getBibStyles(str);
	for (vector<string>::const_iterator it = str.begin();
		it != str.end(); ++it) {
		string item(changeExtension(*it, ""));
		if (item == bibstyle)
			item_nr = int(it - str.begin());
		dialog_->styleCB->addItem(toqstr(item));
	}

	if (item_nr == -1 && !bibstyle.empty()) {
		dialog_->styleCB->addItem(toqstr(bibstyle));
		item_nr = dialog_->styleCB->count() - 1;
	}

	if (item_nr != -1)
		dialog_->styleCB->setCurrentIndex(item_nr);
	else
		dialog_->styleCB->clearEditText();
}


void GuiBibtex::apply()
{
	docstring dbs(qstring_to_ucs4(dialog_->databaseLW->item(0)->text()));

	unsigned int maxCount = dialog_->databaseLW->count();
	for (unsigned int i = 1; i < maxCount; i++) {
		dbs += ',';
		dbs += qstring_to_ucs4(dialog_->databaseLW->item(i)->text());
	}

	controller().params()["bibfiles"] = dbs;

	docstring const bibstyle(qstring_to_ucs4(dialog_->styleCB->currentText()));
	bool const bibtotoc(dialog_->bibtocCB->isChecked());

	if (bibtotoc && (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params()["options"] = "bibtotoc," + bibstyle;
	} else if (bibtotoc) {
		// bibtotoc and no style
		controller().params()["options"] = from_ascii("bibtotoc");
	} else {
		// only style. An empty one is valid, because some
		// documentclasses have an own \bibliographystyle{}
		// command!
		controller().params()["options"] = bibstyle;
	}

	// bibtopic allows three kinds of sections:
	// 1. sections that include all cited references of the database(s)
	// 2. sections that include all uncited references of the database(s)
	// 3. sections that include all references of the database(s), cited or not
	int btp = dialog_->btPrintCO->currentIndex();

	switch (btp) {
	case 0:
		controller().params()["btprint"] = from_ascii("btPrintCited");
		break;
	case 1:
		controller().params()["btprint"] = from_ascii("btPrintNotCited");
		break;
	case 2:
		controller().params()["btprint"] = from_ascii("btPrintAll");
		break;
	}

	if (!controller().usingBibtopic())
		controller().params()["btprint"] = docstring();
}


bool GuiBibtex::isValid()
{
	return dialog_->databaseLW->count() != 0;
}

} // namespace frontend
} // namespace lyx

#include "GuiBibtex_moc.cpp"
