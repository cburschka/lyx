/**
 * \file QBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"

#include "ControlBibtex.h"
#include "qt_helpers.h"
#include "debug.h"

#include "support/filetools.h" // ChangeExtension
#include "support/lstrings.h" // getVectorFromString

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#include "QBibtexDialog.h"
#include "QBibtex.h"
#include "Qt2BC.h"

using std::vector;

typedef Qt2CB<ControlBibtex, Qt2DB<QBibtexDialog> > base_class;


QBibtex::QBibtex()
	: base_class(qt_("BibTeX"))
{
}


void QBibtex::build_dialog()
{
	dialog_.reset(new QBibtexDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->databaseLB);
	bc().addReadOnly(dialog_->databasePB);
	bc().addReadOnly(dialog_->stylePB);
	bc().addReadOnly(dialog_->styleCB);
	bc().addReadOnly(dialog_->bibtocCB);
	bc().addReadOnly(dialog_->databasePB);
	bc().addReadOnly(dialog_->deletePB);
}


void QBibtex::update_contents()
{
	dialog_->databaseLB->clear();

	string bibs(controller().params().getContents());
	string bib;

	while (!bibs.empty()) {
		bibs = split(bibs, bib, ',');
		bib = trim(bib);
		if (!bib.empty())
			dialog_->databaseLB->insertItem(toqstr(bib));
	}

	string bibtotoc = "bibtotoc";
	string bibstyle(toqstr(controller().params().getOptions()));

	// bibtotoc exists?
	if (prefixIs(bibstyle, bibtotoc)) {
		dialog_->bibtocCB->setChecked(true);

		// bibstyle exists?
		if (contains(bibstyle,','))
			bibstyle = split(bibstyle, bibtotoc, ',');
		else
			bibstyle.erase();
	} else
		dialog_->bibtocCB->setChecked(false);


	dialog_->styleCB->clear();

	int item_nr(-1);

	vector<string> const str = getVectorFromString(
		controller().getBibStyles(), "\n");
	for (vector<string>::const_iterator it = str.begin();
		it != str.end(); ++it) {
		string item(ChangeExtension(*it, ""));
		if (item == bibstyle)
			item_nr = int(it - str.begin());
		dialog_->styleCB->insertItem(toqstr(item));
	}

	if (item_nr == -1) {
		dialog_->styleCB->insertItem(toqstr(bibstyle));
		item_nr = dialog_->styleCB->count() - 1;
	}

	dialog_->styleCB->setCurrentItem(item_nr);
}


void QBibtex::apply()
{
	string dbs(fromqstr(dialog_->databaseLB->text(0)));

	unsigned int maxCount = dialog_->databaseLB->count();
	for (unsigned int i = 1; i < maxCount; i++) {
		dbs += ',';
		dbs += fromqstr(dialog_->databaseLB->text(i));
	}

	controller().params().setContents(dbs);

	string const bibstyle(fromqstr(dialog_->styleCB->currentText()));
	bool const bibtotoc(dialog_->bibtocCB->isChecked());

	if (bibtotoc && (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params().setOptions("bibtotoc," + bibstyle);
	} else if (bibtotoc) {
		// bibtotoc and no style
		controller().params().setOptions("bibtotoc");
	} else {
		// only style. An empty one is valid, because some
		// documentclasses have an own \bibliographystyle{}
		// command!
		controller().params().setOptions(bibstyle);
	}
}


bool QBibtex::isValid()
{
	return dialog_->databaseLB->count() != 0;
}
