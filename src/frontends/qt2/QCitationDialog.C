/**
 * \file QCitationDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qt_helpers.h"
#include "controllers/ControlCitation.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#include "ui/QCitationFindDialogBase.h"
#include "QCitationDialog.h"
#include "QCitation.h"

using std::vector;


QCitationDialog::QCitationDialog(QCitation * form)
	: QCitationDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	add_ = new QCitationFindDialogBase(this, "", true);
	connect(add_->previousPB, SIGNAL(clicked()), this, SLOT(previous()));
	connect(add_->nextPB, SIGNAL(clicked()), this, SLOT(next()));
	connect(add_->availableLB, SIGNAL(currentChanged(QListBoxItem *)), this, SLOT(availableChanged()));
	connect(add_->availableLB, SIGNAL(selected(QListBoxItem *)), this, SLOT(addCitation()));
	connect(add_->availableLB, SIGNAL(selected(QListBoxItem *)), add_, SLOT(accept()));
	connect(add_->addPB, SIGNAL(clicked()), this, SLOT(addCitation()));
	connect(selectedLB, SIGNAL(returnPressed(QListBoxItem *)), form, SLOT(slotOK()));
}


QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::setButtons()
{
	if (form_->readOnly())
		return;

	int const sel_nr = selectedLB->currentItem();
	int const avail_nr = add_->availableLB->currentItem();

	add_->addPB->setEnabled(avail_nr >= 0);
	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < int(selectedLB->count() - 1));
}


void QCitationDialog::selectedChanged()
{
	form_->fillStyles();
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();
	infoML->clear();

	int const sel = selectedLB->currentItem();
	if (sel < 0) {
		setButtons();
		return;
	}

	infoML->setText(toqstr(biblio::getInfo(theMap, form_->citekeys[sel])));
	setButtons();
}


void QCitationDialog::previous()
{
	find(biblio::BACKWARD);
}


void QCitationDialog::next()
{
	find(biblio::FORWARD);
}


void QCitationDialog::availableChanged()
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();
	add_->infoML->clear();

	int const sel = add_->availableLB->currentItem();
	if (sel < 0) {
		setButtons();
		return;
	}

	add_->infoML->setText(toqstr(biblio::getInfo(theMap, form_->bibkeys[sel])));
	setButtons();
}


void QCitationDialog::addCitation()
{
	int const sel = add_->availableLB->currentItem();

	if (sel < 0)
		return;

	// Add the selected browser_bib keys to browser_cite
	// multiple selections are possible
	for (unsigned int i = 0; i != add_->availableLB->count(); i++) {
		if (add_->availableLB->isSelected(i)) {
			// do not allow duplicates
			if ((selectedLB->findItem(add_->availableLB->text(i))) == 0) {
				selectedLB->insertItem(toqstr(form_->bibkeys[i]));
				form_->citekeys.push_back(form_->bibkeys[i]);
			}
		}
	}

	int const n = int(form_->citekeys.size());
	selectedLB->setSelected(n - 1, true);

	form_->changed();
	form_->fillStyles();
	setButtons();
}


void QCitationDialog::del()
{
	int const sel = selectedLB->currentItem();

	// Remove the selected key from browser_cite
	selectedLB->removeItem(sel);
	form_->citekeys.erase(form_->citekeys.begin() + sel);

	form_->changed();
	form_->fillStyles();
	setButtons();
}


void QCitationDialog::up()
{
	int const sel = selectedLB->currentItem();

	// Move the selected key up one line
	vector<string>::iterator it = form_->citekeys.begin() + sel;
	string const tmp = *it;

	selectedLB->removeItem(sel);
	form_->citekeys.erase(it);

	selectedLB->insertItem(toqstr(tmp), sel - 1);
	selectedLB->setSelected(sel - 1, true);
	form_->citekeys.insert(it - 1, tmp);

	form_->changed();
	form_->fillStyles();
	setButtons();
}


void QCitationDialog::down()
{
	int const sel = selectedLB->currentItem();

	// Move the selected key down one line
	vector<string>::iterator it = form_->citekeys.begin() + sel;
	string const tmp = *it;

	selectedLB->removeItem(sel);
	form_->citekeys.erase(it);

	selectedLB->insertItem(toqstr(tmp), sel + 1);
	selectedLB->setSelected(sel + 1, true);
	form_->citekeys.insert(it + 1, tmp);

	form_->changed();
	form_->fillStyles();
	setButtons();
}


void QCitationDialog::add()
{
	add_->exec();
}


void QCitationDialog::changed_adaptor()
{
	form_->changed();
}


void QCitationDialog::find(biblio::Direction dir)
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

	biblio::Search const type = add_->searchTypeCB->isChecked()
	       	? biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = form_->bibkeys.begin();
	int const sel = add_->availableLB->currentItem();
	if (sel >= 0 && sel <= int(form_->bibkeys.size()-1))
		start += sel;

	// Find the NEXT instance...
	if (dir == biblio::FORWARD)
		start += 1;
	else
		start -= 1;

	bool const casesens = add_->searchCaseCB->isChecked();
	string const str = fromqstr(add_->searchED->text());

	vector<string>::const_iterator cit =
		biblio::searchKeys(theMap, form_->bibkeys, str,
				   start, type, dir, casesens);

	// not found. let's loop round
	if (cit == form_->bibkeys.end()) {
		if (dir == biblio::FORWARD) {
			start = form_->bibkeys.begin();
		}
		else start = form_->bibkeys.end() - 1;

		cit = biblio::searchKeys(theMap, form_->bibkeys, str,
					 start, type, dir, casesens);

		if (cit == form_->bibkeys.end())
			return;
	}

	int const found = int(cit - form_->bibkeys.begin());
	if (found == sel) {
		return;
	}

	// Update the display
	add_->availableLB->setSelected(found, true);
	add_->availableLB->ensureCurrentVisible();
}
