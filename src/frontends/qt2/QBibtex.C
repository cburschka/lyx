/**
 * \file QBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


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

#include "ui/QBibtexAddDialogBase.h"
#include "QBibtexDialog.h"
#include "QBibtex.h"
#include "Qt2BC.h"

using namespace lyx::support;

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
	InsetBibtexParams const & params = controller().params();

	dialog_->databaseLB->clear();

	vector<FileName>::const_iterator fit  = params.databases.begin();
	vector<FileName>::const_iterator fend = params.databases.end();
	for (; fit != fend; ++fit) {
		string const db = fit->outputFilename(kernel().bufferFilepath());
		dialog_->databaseLB->insertItem(toqstr(db));
	}

	dialog_->add_->bibLB->clear();

	vector<string> bib_str;
	controller().getBibFiles(bib_str);
	vector<string>::const_iterator sit  = bib_str.begin();
	vector<string>::const_iterator send = bib_str.end();
	for (; sit != send; ++sit) {
		string const bibItem = ChangeExtension(*sit, "");
		dialog_->add_->bibLB->insertItem(toqstr(bibItem));
	}

	dialog_->bibtocCB->setChecked(params.bibtotoc);

	dialog_->styleCB->clear();

	int item_nr(-1);

	vector<string> str;
	controller().getBibStyles(str);
	sit  = str.begin();
	send = str.end();
	for (; sit != send; ++sit) {
		string const item = ChangeExtension(*sit, "");
		if (item == params.style)
			item_nr = int(sit - str.begin());
		dialog_->styleCB->insertItem(toqstr(item));
	}

	if (item_nr == -1) {
		dialog_->styleCB->insertItem(toqstr(params.style));
		item_nr = dialog_->styleCB->count() - 1;
	}

	dialog_->styleCB->setCurrentItem(item_nr);
}


void QBibtex::apply()
{
	InsetBibtexParams params;

        for (unsigned int i = 0; i < dialog_->databaseLB->count(); ++i) {
		FileName file;
		file.set(fromqstr(dialog_->databaseLB->text(i)),
			 kernel().bufferFilepath());
                params.databases.push_back(file);
        }

	params.style = fromqstr(dialog_->styleCB->currentText());
	params.bibtotoc = dialog_->bibtocCB->isChecked();

	controller().params() = params;
}


bool QBibtex::isValid()
{
	return dialog_->databaseLB->count() != 0;
}
