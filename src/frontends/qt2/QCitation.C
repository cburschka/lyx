/**
 * \file QCitation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QCitationDialog.h"
#include "QCitation.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#include "QtLyXView.h"
#include "Qt2BC.h"
#include "ControlCitation.h"
#include "debug.h"
#include "gettext.h"
#include "support/lstrings.h"
#include "helper_funcs.h"


using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;

typedef Qt2CB<ControlCitation, Qt2DB<QCitationDialog> > base_class;


QCitation::QCitation()
	: base_class(_("Citation"))
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

	string const after = dialog_->textAfterED->text().latin1();
	controller().params().setOptions(after);
}


void QCitation::hide()
{
	citekeys.clear();
	bibkeys.clear();

	Qt2Base::hide();
}


void QCitation::build_dialog()
{
	dialog_.reset(new QCitationDialog(this));

	dialog_->searchTypeCB->setChecked(false);
	dialog_->searchCaseCB->setChecked(false);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);

	bc().addReadOnly(dialog_->addPB);
	bc().addReadOnly(dialog_->delPB);
	bc().addReadOnly(dialog_->upPB);
	bc().addReadOnly(dialog_->downPB);
	bc().addReadOnly(dialog_->citationStyleCO);
	bc().addReadOnly(dialog_->forceuppercaseCB);
	bc().addReadOnly(dialog_->fulllistCB);
	// add when enabled !
	//bc().addReadOnly(dialog_->textBeforeED);
	bc().addReadOnly(dialog_->textAfterED);
}


void QCitation::fillStyles()
{
	// style
	string key;

	if (!citekeys.empty())
		key = citekeys[0];

	dialog_->citationStyleCO->clear();
	vector<string> const & sty = controller().getCiteStrings(key);

	for (vector<string>::const_iterator it = sty.begin();
		it != sty.end(); ++it) {
		dialog_->citationStyleCO->insertItem(it->c_str());
	}
}


void QCitation::updateStyle()
{
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

	bool const natbib = controller().usingNatbib();
	dialog_->citationStyleCO->setEnabled(natbib);
	dialog_->fulllistCB->setEnabled(natbib);
	dialog_->forceuppercaseCB->setEnabled(natbib);
}


void QCitation::update_contents()
{
	// Make the list of all available bibliography keys
	bibkeys = biblio::getKeys(controller().bibkeysInfo());
	updateBrowser(dialog_->bibLB, bibkeys);

	// Ditto for the keys cited in this inset
	citekeys = getVectorFromString(controller().params().getContents());
	updateBrowser(dialog_->citeLB, citekeys);

	// No keys have been selected yet, so...
	dialog_->infoML->clear();
	setBibButtons(OFF);
	setCiteButtons(OFF);

	dialog_->textAfterED->setText(controller().params().getOptions().c_str());

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
			browser->insertItem(key.c_str());
	}
}


void QCitation::setBibButtons(State status) const
{
	dialog_->addPB->setEnabled((status == ON));
}


void QCitation::setCiteButtons(State status) const
{
	int const sel = dialog_->citeLB->currentItem();
	int const maxline = dialog_->citeLB->count() - 1;
	bool const activate = (status == ON);
	bool const activate_up = (activate && sel != 0);
	bool const activate_down = (activate && sel != maxline);

	dialog_->delPB->setEnabled(activate);
	dialog_->upPB->setEnabled(activate_up);
	dialog_->downPB->setEnabled(activate_down);
}
