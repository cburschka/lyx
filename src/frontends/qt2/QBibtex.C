/**
 * \file QBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "debug.h"
#include "support/lstrings.h"

#include "ControlBibtex.h"
#include "qt_helpers.h"

#include "support/filetools.h" // ChangeExtension

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#include "ui/QBibtexAddDialogBase.h"
#include "QBibtexDialog.h"
#include "QBibtex.h"
#include "Qt2BC.h"

using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::prefixIs;
using lyx::support::split;
using lyx::support::trim;

using std::vector;


typedef QController<ControlBibtex, QView<QBibtexDialog> > base_class;

QBibtex::QBibtex(Dialog & parent)
	: base_class(parent, _("BibTeX"))
{
}


void QBibtex::build_dialog()
{
	dialog_.reset(new QBibtexDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->databaseLB);
	bcview().addReadOnly(dialog_->stylePB);
	bcview().addReadOnly(dialog_->styleCB);
	bcview().addReadOnly(dialog_->bibtocCB);
	bcview().addReadOnly(dialog_->addBibPB);
	bcview().addReadOnly(dialog_->deletePB);
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

	dialog_->add_->bibLB->clear();

	vector<string> bib_str;
	controller().getBibFiles(bib_str);
	for (vector<string>::const_iterator it = bib_str.begin();
		it != bib_str.end(); ++it) {
		string bibItem(ChangeExtension(*it, ""));
		dialog_->add_->bibLB->insertItem(toqstr(bibItem));
	}

	string bibtotoc = "bibtotoc";
	string bibstyle(controller().params().getOptions());

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

	vector<string> str;
	controller().getBibStyles(str);
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
