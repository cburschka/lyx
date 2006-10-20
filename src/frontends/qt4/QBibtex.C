/**
 * \file QBibtex.C
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

#include "QBibtex.h"
#include "QBibtexDialog.h"
#include "ui/QBibtexAddUi.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "validators.h"

#include "lyxrc.h"

#include "controllers/ControlBibtex.h"

#include "support/filetools.h" // changeExtension
#include "support/lstrings.h"

#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>


using lyx::support::changeExtension;
using lyx::support::split;
using lyx::support::trim;

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlBibtex, QView<QBibtexDialog> > base_class;

QBibtex::QBibtex(Dialog & parent)
	: base_class(parent, _("BibTeX Bibliography"))
{
}


void QBibtex::build_dialog()
{
	dialog_.reset(new QBibtexDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->databaseLW);
	bcview().addReadOnly(dialog_->stylePB);
	bcview().addReadOnly(dialog_->styleCB);
	bcview().addReadOnly(dialog_->bibtocCB);
	bcview().addReadOnly(dialog_->addBibPB);
	bcview().addReadOnly(dialog_->deletePB);
}


void QBibtex::update_contents()
{
	PathValidator * path_validator =
		getPathValidator(dialog_->add_->bibED);
	if (path_validator)
		path_validator->setChecker(kernel().docType(), lyxrc);

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


void QBibtex::apply()
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
		controller().params()["options"] = lyx::from_ascii("bibtotoc");
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
		controller().params()["btprint"] = lyx::from_ascii("btPrintCited");
		break;
	case 1:
		controller().params()["btprint"] = lyx::from_ascii("btPrintNotCited");
		break;
	case 2:
		controller().params()["btprint"] = lyx::from_ascii("btPrintAll");
		break;
	}

	if (!controller().usingBibtopic())
		controller().params()["btprint"] = docstring();
}


bool QBibtex::isValid()
{
	return dialog_->databaseLW->count() != 0;
}

} // namespace frontend
} // namespace lyx
