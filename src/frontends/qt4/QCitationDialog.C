/**
 * \file QCitationDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCitationDialog.h"
#include "ui/QCitationFindUi.h"
#include "QCitation.h"
#include "qt_helpers.h"

#include "controllers/ControlCitation.h"
#include "controllers/ButtonController.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <q3multilineedit.h>
#include <qpushbutton.h>


using std::vector;
using std::string;

namespace lyx {
namespace frontend {

QCitationDialog::QCitationDialog(QCitation * form)
	: form_(form)
{
	setupUi(this);
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( citationStyleCO, SIGNAL( activated(int) ), this, SLOT( changed_adaptor() ) );
    connect( fulllistCB, SIGNAL( clicked() ), this, SLOT( changed_adaptor() ) );
    connect( forceuppercaseCB, SIGNAL( clicked() ), this, SLOT( changed_adaptor() ) );
    connect( textBeforeED, SIGNAL( textChanged(const QString&) ), this, SLOT( changed_adaptor() ) );
    connect( textAfterED, SIGNAL( textChanged(const QString&) ), this, SLOT( changed_adaptor() ) );
    connect( upPB, SIGNAL( clicked() ), this, SLOT( up() ) );
    connect( downPB, SIGNAL( clicked() ), this, SLOT( down() ) );
    connect( selectedLB, SIGNAL( currentChanged(QListBoxItem*) ), this, SLOT( selectedChanged() ) );
    connect( addPB, SIGNAL( clicked() ), this, SLOT( add() ) );
    connect( deletePB, SIGNAL( clicked() ), this, SLOT( del() ) );

	add_ = new QDialog(this, "", true);
	ui_.setupUi(add_);

    connect( ui_.addPB, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( ui_.closePB, SIGNAL( clicked() ), this, SLOT( reject() ) );

	connect(ui_.previousPB, SIGNAL(clicked()), this, SLOT(previous()));
	connect(ui_.nextPB, SIGNAL(clicked()), this, SLOT(next()));
	connect(ui_.availableLB, SIGNAL(currentChanged(Q3ListBoxItem *)), this, SLOT(availableChanged()));
	connect(ui_.availableLB, SIGNAL(selected(Q3ListBoxItem *)), this, SLOT(addCitation()));
	connect(ui_.availableLB, SIGNAL(selected(Q3ListBoxItem *)), add_, SLOT(accept()));
	connect(ui_.addPB, SIGNAL(clicked()), this, SLOT(addCitation()));
	connect(selectedLB, SIGNAL(returnPressed(Q3ListBoxItem *)), form, SLOT(slotOK()));
}


QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::setButtons()
{
	if (form_->readOnly())
		return;

	int const sel_nr = selectedLB->currentItem();
	int const avail_nr = ui_.availableLB->currentItem();

	ui_.addPB->setEnabled(avail_nr >= 0);
	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < int(selectedLB->count() - 1));
}


void QCitationDialog::openFind()
{
	if (form_->readOnly())
		return;

	if (isVisible() && selectedLB->count() == 0 
	    && ui_.availableLB->count() != 0){
		// open the find dialog
		add();
		// and let the user press ok after a selection
		if (selectedLB->count() != 0)
			form_->bc().valid();
	}
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

	if (!theMap.empty())
		infoML->setText(
			toqstr(biblio::getInfo(theMap, form_->citekeys[sel])));
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
	ui_.infoML->clear();

	int const sel = ui_.availableLB->currentItem();
	if (sel < 0) {
		setButtons();
		return;
	}

	if (!theMap.empty())
		ui_.infoML->setText(
			toqstr(biblio::getInfo(theMap, form_->bibkeys[sel])));
	setButtons();
}


void QCitationDialog::addCitation()
{
	int const sel = ui_.availableLB->currentItem();

	if (sel < 0)
		return;

	// Add the selected browser_bib keys to browser_cite
	// multiple selections are possible
	for (unsigned int i = 0; i != ui_.availableLB->count(); i++) {
		if (ui_.availableLB->isSelected(i)) {
			// do not allow duplicates
			if ((selectedLB->findItem(ui_.availableLB->text(i))) == 0) {
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

	biblio::Search const type = ui_.searchTypeCB->isChecked()
		? biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = form_->bibkeys.begin();
	int const sel = ui_.availableLB->currentItem();
	if (sel >= 0 && sel <= int(form_->bibkeys.size()-1))
		start += sel;

	// Find the NEXT instance...
	if (dir == biblio::FORWARD)
		start += 1;

	bool const casesens = ui_.searchCaseCB->isChecked();
	string const str = fromqstr(ui_.searchED->text());

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
	// note that we have multi selection mode!
	ui_.availableLB->setSelected(sel, false);
	ui_.availableLB->setSelected(found, true);
	ui_.availableLB->setCurrentItem(found);
	ui_.availableLB->ensureCurrentVisible();
}

} // namespace frontend
} // namespace lyx
