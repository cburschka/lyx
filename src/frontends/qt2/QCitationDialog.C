/**
 * \file QCitationDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include <algorithm>

#include "qt_helpers.h"
#include "controllers/ControlCitation.h"
#include "debug.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "QCitationDialog.h"
#include "QCitation.h"
#include "support/lstrings.h"

using std::vector;
using std::find;
using std::max;
using std::endl;


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
}


QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::setButtons()
{
	if (form_->readOnly())
		return;

	int const sel_nr = selectedLB->currentItem();
	int const avail_nr = availableLB->currentItem();

	addPB->setEnabled(avail_nr >= 0);
	delPB->setEnabled(sel_nr >= 0);
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

	vector<string>::const_iterator cit =
		std::find(form_->bibkeys.begin(),
		form_->bibkeys.end(), form_->citekeys[sel]);

	if (cit != form_->bibkeys.end()) {
		int const n = int(cit - form_->bibkeys.begin());
		availableLB->setSelected(n, true);
		availableLB->ensureCurrentVisible();
	}
	setButtons();
}


void QCitationDialog::availableChanged()
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();
	selectedLB->clearSelection();
	infoML->clear();

	int const sel = availableLB->currentItem();
	if (sel < 0) {
		setButtons();
		return;
	}

	infoML->setText(toqstr(biblio::getInfo(theMap, form_->bibkeys[sel])));

	vector<string>::const_iterator cit =
		std::find(form_->citekeys.begin(), form_->citekeys.end(),
			  form_->bibkeys[sel]);

	if (cit != form_->citekeys.end()) {
		int const n = int(cit - form_->citekeys.begin());
		selectedLB->setSelected(n, true);
		selectedLB->ensureCurrentVisible();
	}
	setButtons();
}


void QCitationDialog::add()
{
	int const sel = availableLB->currentItem();

	// Add the selected browser_bib key to browser_cite
	selectedLB->insertItem(toqstr(form_->bibkeys[sel]));
	form_->citekeys.push_back(form_->bibkeys[sel]);

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
	availableLB->clearSelection();
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
	availableLB->clearSelection();
	setButtons();
}


void QCitationDialog::previous()
{
	doFind(biblio::BACKWARD);
}


void QCitationDialog::next()
{
	doFind(biblio::FORWARD);
}


void QCitationDialog::changed_adaptor()
{
	form_->changed();
}


void QCitationDialog::doFind(biblio::Direction dir)
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();
	string const str = fromqstr(searchED->text());

	biblio::Search const type =
		searchTypeCB->isChecked() ?
		biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = form_->bibkeys.begin();
	int const sel = availableLB->currentItem();
	if (sel >= 0 && sel <= int(form_->bibkeys.size()-1))
		start += sel;

	// Find the NEXT instance...
	if (dir == biblio::FORWARD)
		start += 1;
	else
		start -= 1;

	bool const caseSensitive = searchCaseCB->isChecked();

	vector<string>::const_iterator cit =
		biblio::searchKeys(theMap, form_->bibkeys, str,
				   start, type, dir, caseSensitive);

	// not found. let's loop round
	if (cit == form_->bibkeys.end()) {
		if (dir == biblio::FORWARD) {
			start = form_->bibkeys.begin();
		}
		else start = form_->bibkeys.end() - 1;

		cit = biblio::searchKeys(theMap, form_->bibkeys, str,
					 start, type, dir, caseSensitive);

		if (cit == form_->bibkeys.end())
			return;
	}

	int const found = int(cit - form_->bibkeys.begin());
	if (found == sel) {
		return;
	}

	// Update the display
	availableLB->setSelected(found, true);
	availableLB->ensureCurrentVisible();
}
