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

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>

#include "gettext.h"
#include "controllers/ControlCitation.h"
#include "LyXView.h"
#include "buffer.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#include "QCitationDialog.h"
#include "QCitation.h"
#include "support/lstrings.h"

using std::vector;
using std::find;
using std::max;

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
	connect(searchED, SIGNAL(returnPressed()),
		this, SLOT(slotNextClicked()));

	textBeforeED->setText(_("Not yet supported"));
	textBeforeED->setReadOnly(true);
	textBeforeED->setFocusPolicy(QWidget::NoFocus);
	citationStyleCO->setEnabled(false);
	citationStyleCO->setFocusPolicy(QWidget::NoFocus);
}


QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::slotBibSelected(int sel)
{
	slotBibHighlighted(sel);

	if (form_->readOnly())
		return;

	slotAddClicked();
}


void QCitationDialog::slotBibHighlighted(int sel)
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

	citeLB->clearSelection();

	// FIXME: why would this happen ?
	if (sel < 0 || sel >= (int)form_->bibkeys.size()) {
		return;
	}

	// Put into browser_info the additional info associated with
	// the selected browser_bib key
	infoML->clear();

	infoML->setText(biblio::getInfo(theMap, form_->bibkeys[sel]).c_str());

	// Highlight the selected browser_bib key in browser_cite if
	// present
	vector<string>::const_iterator cit =
		std::find(form_->citekeys.begin(), form_->citekeys.end(),
			  form_->bibkeys[sel]);

	if (cit != form_->citekeys.end()) {
		int const n = int(cit - form_->citekeys.begin());
		citeLB->setSelected(n, true);
		citeLB->setTopItem(n);
	}

	if (!form_->readOnly()) {
		if (cit != form_->citekeys.end()) {
			form_->setBibButtons(QCitation::OFF);
			form_->setCiteButtons(QCitation::ON);
		} else {
			form_->setBibButtons(QCitation::ON);
			form_->setCiteButtons(QCitation::OFF);
		}
	}
}


void QCitationDialog::slotCiteHighlighted(int sel)
{
	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

	// FIXME: why would this happen ?
	if (sel < 0 || sel >= (int)form_->citekeys.size()) {
		return;
	}

	if (!form_->readOnly()) {
		form_->setBibButtons(QCitation::OFF);
		form_->setCiteButtons(QCitation::ON);
	}

	// Highlight the selected browser_cite key in browser_bib
	vector<string>::const_iterator cit =
		std::find(form_->bibkeys.begin(),
		form_->bibkeys.end(), form_->citekeys[sel]);

	if (cit != form_->bibkeys.end()) {
		int const n = int(cit - form_->bibkeys.begin());
		bibLB->setSelected(n, true);
		bibLB->setTopItem(n);

		// Put into browser_info the additional info associated
		// with the selected browser_cite key
		infoML->clear();
		infoML->setText(biblio::getInfo(theMap, form_->citekeys[sel]).c_str());
	}
}


void QCitationDialog::slotAddClicked()
{
	int const sel = bibLB->currentItem();

	// FIXME: why ?
	if (sel < 0 || sel >= (int)form_->bibkeys.size()) {
		return;
	}

	// Add the selected browser_bib key to browser_cite
	citeLB->insertItem(form_->bibkeys[sel].c_str());
	form_->citekeys.push_back(form_->bibkeys[sel]);

	int const n = int(form_->citekeys.size());
	citeLB->setSelected(n - 1, true);

	slotBibHighlighted(sel);
	form_->setBibButtons(QCitation::OFF);
	form_->setCiteButtons(QCitation::ON);
	form_->changed();
}


void QCitationDialog::slotDelClicked()
{
	int const sel = citeLB->currentItem();

	// FIXME: why ?
	if (sel < 0 || sel >= (int)form_->citekeys.size()) {
		return;
	}

	// Remove the selected key from browser_cite
	citeLB->removeItem(sel);
	form_->citekeys.erase(form_->citekeys.begin() + sel);

	form_->setBibButtons(QCitation::ON);
	form_->setCiteButtons(QCitation::OFF);
	form_->changed();
}


void QCitationDialog::slotUpClicked()
{
	int const sel = citeLB->currentItem();

	// FIXME: why ?
	if (sel < 1 || sel >= (int)form_->citekeys.size()) {
		return;
	}

	// Move the selected key up one line
	vector<string>::iterator it = form_->citekeys.begin() + sel;
	string const tmp = *it;

	citeLB->removeItem(sel);
	form_->citekeys.erase(it);

	citeLB->insertItem(tmp.c_str(), sel - 1);
	citeLB->setSelected(sel - 1, true);
	form_->citekeys.insert(it - 1, tmp);
	form_->setCiteButtons(QCitation::ON);
	form_->changed();
}


void QCitationDialog::slotDownClicked()
{
	int const sel = citeLB->currentItem();

	// FIXME: ?
	if (sel < 0 || sel >= (int)form_->citekeys.size() - 1) {
		return;
	}

	// Move the selected key down one line
	vector<string>::iterator it = form_->citekeys.begin() + sel;
	string const tmp = *it;

	citeLB->removeItem(sel);
	form_->citekeys.erase(it);

	citeLB->insertItem(tmp.c_str(), sel + 1);
	citeLB->setSelected(sel + 1, true);
	form_->citekeys.insert(it + 1, tmp);
	form_->setCiteButtons(QCitation::ON);
	form_->changed();
}


void QCitationDialog::slotPreviousClicked()
{
	doFind(biblio::BACKWARD);
}


void QCitationDialog::slotNextClicked()
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
	string const str = searchED->text().latin1();

	biblio::Search const type =
		searchTypeCB->isChecked() ?
		biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = form_->bibkeys.begin();
	int const sel = bibLB->currentItem();
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
	int const top = max(found - 5, 1);
	bibLB->setTopItem(top);
	bibLB->setSelected(found, true);
	slotBibHighlighted(0);
}
