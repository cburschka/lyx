/**
 * \file QCitation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "ui/QCitationFindDialogBase.h"
#include "QCitationDialog.h"
#include "QCitation.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "Qt2BC.h"
#include "ControlCitation.h"
#include "qt_helpers.h"
#include "support/lstrings.h"

using lyx::support::getStringFromVector;
using lyx::support::getVectorFromString;
using lyx::support::trim;

using std::find;

using std::vector;


typedef QController<ControlCitation, QView<QCitationDialog> > base_class;

QCitation::QCitation(Dialog & parent)
	: base_class(parent, _("LyX: Citation Reference"))
{}


void QCitation::apply()
{
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	int const choice = dialog_->citationStyleCO->currentItem();
	bool const full  = dialog_->fulllistCB->isChecked();
	bool const force = dialog_->forceuppercaseCB->isChecked();

	string const command =
		biblio::getCiteCommand(styles[choice], full, force);

	controller().params().setCmdName(command);
	controller().params().setContents(getStringFromVector(citekeys));

	string const after = fromqstr(dialog_->textAfterED->text());
	controller().params().setOptions(after);
}


void QCitation::hide()
{
	citekeys.clear();
	bibkeys.clear();

	QDialogView::hide();
}


void QCitation::build_dialog()
{
	dialog_.reset(new QCitationDialog(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);

	bcview().addReadOnly(dialog_->addPB);
	bcview().addReadOnly(dialog_->deletePB);
	bcview().addReadOnly(dialog_->upPB);
	bcview().addReadOnly(dialog_->downPB);
	bcview().addReadOnly(dialog_->citationStyleCO);
	bcview().addReadOnly(dialog_->forceuppercaseCB);
	bcview().addReadOnly(dialog_->fulllistCB);
	// add when enabled !
	//bcview().addReadOnly(dialog_->textBeforeED);
	bcview().addReadOnly(dialog_->textAfterED);
}


void QCitation::fillStyles()
{
	if (citekeys.empty()) {
		dialog_->citationStyleCO->setEnabled(false);
		dialog_->citationStyleLA->setEnabled(false);
		return;
	}

	int const orig = dialog_->citationStyleCO->currentItem();

	dialog_->citationStyleCO->clear();

	int curr = dialog_->selectedLB->currentItem();
	if (curr < 0)
		curr = 0;

	string key = citekeys[curr];

	vector<string> const & sty = controller().getCiteStrings(key);

	bool const natbib = controller().usingNatbib();
	dialog_->citationStyleCO->setEnabled(!sty.empty() && natbib);
	dialog_->citationStyleLA->setEnabled(!sty.empty() && natbib);

	for (vector<string>::const_iterator it = sty.begin();
		it != sty.end(); ++it) {
		dialog_->citationStyleCO->insertItem(toqstr(*it));
	}

	if (orig != -1 && orig < dialog_->citationStyleCO->count())
		dialog_->citationStyleCO->setCurrentItem(orig);
}


void QCitation::updateStyle()
{
	bool const natbib = controller().usingNatbib();

	dialog_->fulllistCB->setEnabled(natbib);
	dialog_->forceuppercaseCB->setEnabled(natbib);

	string const & command = controller().params().getCmdName();

	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();
	biblio::CitationStyle cs = biblio::getCitationStyle(command);

	vector<biblio::CiteStyle>::const_iterator cit =
		find(styles.begin(), styles.end(), cs.style);

	dialog_->citationStyleCO->setCurrentItem(0);
	dialog_->fulllistCB->setChecked(false);
	dialog_->forceuppercaseCB->setChecked(false);

	if (cit != styles.end()) {
		int const i = int(cit - styles.begin());
		dialog_->citationStyleCO->setCurrentItem(i);
		dialog_->fulllistCB->setChecked(cs.full);
		dialog_->forceuppercaseCB->setChecked(cs.forceUCase);
	}
}


void QCitation::update_contents()
{
	// Make the list of all available bibliography keys
	bibkeys = biblio::getKeys(controller().bibkeysInfo());
	updateBrowser(dialog_->add_->availableLB, bibkeys);

	// Ditto for the keys cited in this inset
	citekeys = getVectorFromString(controller().params().getContents());
	updateBrowser(dialog_->selectedLB, citekeys);

	// No keys have been selected yet, so...
	dialog_->infoML->clear();
	dialog_->setButtons();

	dialog_->textAfterED->setText(toqstr(controller().params().getOptions()));

	fillStyles();
	updateStyle();
}


void QCitation::updateBrowser(QListBox * browser,
                              vector<string> const & keys) const
{
	browser->clear();

	for (vector<string>::const_iterator it = keys.begin();
		it < keys.end(); ++it) {
		string const key = trim(*it);
		// FIXME: why the .empty() test ?
		if (!key.empty())
			browser->insertItem(toqstr(key));
	}
}
