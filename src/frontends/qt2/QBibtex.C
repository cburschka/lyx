/**
 * \file QBibtex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/lstrings.h"

#include "ControlBibtex.h"
#include "gettext.h"
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

QBibtex::QBibtex(ControlBibtex & c, Dialogs &)
	: base_class(c, _("BibTeX"))
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
			dialog_->databaseLB->inSort(bib.c_str());
	}

	string bibtotoc = "bibtotoc";
	string bibstyle(controller().params().getOptions().c_str());

	// bibtotoc exists?
	if (prefixIs(bibstyle,bibtotoc)) {
		dialog_->bibtocCB->setChecked(true);

		// bibstyle exists?
		if (contains(bibstyle,','))
			bibstyle = split(bibstyle, bibtotoc, ',');
		else
			bibstyle = "";
	} else
		dialog_->bibtocCB->setChecked(false);


	vector<string> const str = getVectorFromString(
		controller().getBibStyles(),"\n");
	for (vector<string>::const_iterator it = str.begin();
		it != str.end(); ++it) {
		dialog_->styleCB->insertItem(ChangeExtension(*it,"").c_str());
	}
	dialog_->styleCB->insertItem(bibstyle.c_str(),0);
}


void QBibtex::apply()
{
	string dbs;

	for (unsigned int i = 0; i < dialog_->databaseLB->count(); ++i) {
		dbs += dialog_->databaseLB->text(i).latin1();
		if (i != dialog_->databaseLB->count())
			dbs += ",";
	}
	controller().params().setContents(dbs);

	string bibstyle(dialog_->styleCB->text(0).latin1());

	bool const bibtotoc(dialog_->bibtocCB->isChecked());

	if (bibtotoc && (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params().setOptions("bibtotoc," + bibstyle);
	} else if (bibtotoc) {
		// bibtotoc and no style
		controller().params().setOptions("bibtotoc");
	} else if (!bibstyle.empty()){
		// only style
		controller().params().setOptions(bibstyle);
	}
}


bool QBibtex::isValid()
{
	return dialog_->databaseLB->count() != 0 &&
		!string(dialog_->styleCB->text(0)).empty();
}
