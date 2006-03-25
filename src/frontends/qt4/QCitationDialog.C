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
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "bufferparams.h"

#include "controllers/ControlCitation.h"
#include "controllers/ButtonController.h"

#include "support/lstrings.h"

#include <iostream>
using std::cout;
using std::endl;

using std::find;
using std::string;
using std::vector;


namespace lyx {

using support::getStringFromVector;
using support::getVectorFromString;
using support::trim;

namespace frontend {

void updateBrowser(Q3ListBox * browser,
			      vector<string> const & keys)
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

QCitationDialog::QCitationDialog(QCitation * form)
	: form_(form)
{
	setupUi(this);
	
/*	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
*/
	// Manage the ok, apply, restore and cancel/close buttons
	form_->bcview().setOK(okPB);
	form_->bcview().setApply(applyPB);
	form_->bcview().setCancel(closePB);
	form_->bcview().setRestore(restorePB);

	form_->bcview().addReadOnly(addPB);
	form_->bcview().addReadOnly(deletePB);
	form_->bcview().addReadOnly(upPB);
	form_->bcview().addReadOnly(downPB);
	form_->bcview().addReadOnly(citationStyleCO);
	form_->bcview().addReadOnly(forceuppercaseCB);
	form_->bcview().addReadOnly(fulllistCB);
	form_->bcview().addReadOnly(textBeforeED);
	form_->bcview().addReadOnly(textAfterED);

	selectedLV->setModel(form_->selected());
	availableLV->setModel(form_->available());

//	foundLV.setModel(form_->found());

    connect( citationStyleCO, SIGNAL( activated(int) ), this, SLOT( changed() ) );
    connect( fulllistCB, SIGNAL( clicked() ), this, SLOT( changed() ) );
    connect( forceuppercaseCB, SIGNAL( clicked() ), this, SLOT( changed() ) );
    connect( textBeforeED, SIGNAL( textChanged(const QString&) ), this, SLOT( changed() ) );
    connect( textAfterED, SIGNAL( textChanged(const QString&) ), this, SLOT( changed() ) );


//	find_ = new QCitationFind(form_, this);
	
//	connect(selectedLV, SIGNAL(doubleClicked(const QModelIndex & index)),
//		form_, SLOT(on_okPB_clicked()));//SLOT(slotOK()));
}

QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::on_okPB_clicked()
{
	apply(form_->controller().params());
	accept();
}

void QCitationDialog::on_cancelPB_clicked()
{
	reject();
}

void QCitationDialog::on_applyPB_clicked()
{
	apply(form_->controller().params());
}

void QCitationDialog::on_restorePB_clicked()
{
	form_->update_contents();
}

void QCitationDialog::apply(InsetCommandParams & params)
{
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	int  const choice = std::max(0, citationStyleCO->currentItem());
	bool const full  = fulllistCB->isChecked();
	bool const force = forceuppercaseCB->isChecked();

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	params.setCmdName(command);

	string const before = fromqstr(textBeforeED->text());
	params.setSecOptions(before);

	string const after = fromqstr(textAfterED->text());
	params.setOptions(after);

	style_ = choice;
}


void QCitationDialog::update(InsetCommandParams const & params)
{
	// No keys have been selected yet, so...
	infoML->document()->clear();
	setButtons();

	textBeforeED->setText(
		toqstr(params.getSecOptions()));
	textAfterED->setText(
		toqstr(params.getOptions()));

	fillStyles();
	updateStyle();

//	find_->update();
}

void QCitationDialog::updateStyle()
{
	biblio::CiteEngine const engine = form_->controller().getEngine();
	bool const natbib_engine =
		engine == biblio::ENGINE_NATBIB_AUTHORYEAR ||
		engine == biblio::ENGINE_NATBIB_NUMERICAL;
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	fulllistCB->setEnabled(natbib_engine);
	forceuppercaseCB->setEnabled(natbib_engine);
	textBeforeED->setEnabled(!basic_engine);

	string const & command = form_->controller().params().getCmdName();

	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();
	biblio::CitationStyle const cs(command);

	vector<biblio::CiteStyle>::const_iterator cit =
		std::find(styles.begin(), styles.end(), cs.style);

	// restore the latest natbib style
	if (style_ >= 0 && style_ < citationStyleCO->count())
		citationStyleCO->setCurrentItem(style_);
	else
		citationStyleCO->setCurrentItem(0);
	fulllistCB->setChecked(false);
	forceuppercaseCB->setChecked(false);

	if (cit != styles.end()) {
		int const i = int(cit - styles.begin());
		citationStyleCO->setCurrentItem(i);
		fulllistCB->setChecked(cs.full);
		forceuppercaseCB->setChecked(cs.forceUCase);
	}
}


void QCitationDialog::fillStyles()
{
	if (citekeys.empty()) {
		citationStyleCO->setEnabled(false);
		citationStyleLA->setEnabled(false);
		return;
	}

	int const orig = citationStyleCO->currentItem();

	citationStyleCO->clear();

	QStringList selected_keys = form_->selected()->stringList();
	if (selected_keys.empty())
		return;

	if (selectedLV->selectionModel()->selectedIndexes().empty())
		return;
	
	int curr = selectedLV->selectionModel()->selectedIndexes()[0].row();//selectedLV->currentItem();

	string key = fromqstr(selected_keys[curr]);

	vector<string> const & sty = form_->controller().getCiteStrings(key);

	biblio::CiteEngine const engine = form_->controller().getEngine();
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	citationStyleCO->setEnabled(!sty.empty() && !basic_engine);
	citationStyleLA->setEnabled(!sty.empty() && !basic_engine);

	for (vector<string>::const_iterator it = sty.begin();
		it != sty.end(); ++it) {
		citationStyleCO->insertItem(toqstr(*it));
	}

	if (orig != -1 && orig < citationStyleCO->count())
		citationStyleCO->setCurrentItem(orig);
}


void QCitationDialog::setButtons()
{
	if (form_->readOnly())
		return;

	int const row_count = selectedLV->model()->rowCount();

	int sel_nr=-1;
	if (! selectedLV->selectionModel()->selectedIndexes().empty()) {
		sel_nr = 
		selectedLV->selectionModel()->selectedIndexes()[0].row();
	}

	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < row_count - 1);
}

/*
void QCitationDialog::on_selectedLV_currentChanged(Q3ListBoxItem*)
{
	fillStyles();
	infoML->document()->clear();

	int const sel = selectedLB->currentItem();
	if (sel < 0) {
		setButtons();
		return;
	}

	infoML->document()->setPlainText(form_->getKeyInfo(sel));

	setButtons();
}
*/


void QCitationDialog::on_addPB_clicked()
{
	form_->addKeys(availableLV->selectionModel()->selectedIndexes());
}

void QCitationDialog::on_deletePB_clicked()
{
	form_->addKeys(selectedLV->selectionModel()->selectedIndexes());
	changed();
}


void QCitationDialog::on_upPB_clicked()
{
	form_->upKey(selectedLV->selectionModel()->selectedIndexes());
	changed();
}


void QCitationDialog::on_downPB_clicked()
{
	form_->downKey(selectedLV->selectionModel()->selectedIndexes());
	changed();
}

void QCitationDialog::on_findLE_textChanged(const QString & text)
{
	QModelIndex const index = form_->findKey(text);
	if (! index.isValid())
		return;
//	QItemSelection selection(index, index);
	availableLV->selectionModel()->select(index, QItemSelectionModel::Select);
	changed();
}

void QCitationDialog::on_advancedSearchPB_clicked()
{
//	find_->exec();
	changed();
}


void QCitationDialog::changed()
{
	fillStyles();
	setButtons();
}



QCitationFind::QCitationFind(QCitation * form, QWidget * parent, Qt::WFlags f)
: form_(form), QDialog(parent, f)
{
	setupUi(this);
    connect(addPB, SIGNAL(clicked()), this, SLOT(accept()));
    connect(closePB, SIGNAL(clicked()), this, SLOT(reject()));
	connect(previousPB, SIGNAL(clicked()), this, SLOT(previous()));
	connect(nextPB, SIGNAL(clicked()), this, SLOT(next()));
}

void QCitationFind::update()
{
//	updateBrowser(availableLB, form_->availableKeys());
}

void QCitationFind::on_availableLB_currentChanged(Q3ListBoxItem *)
{
	infoML->document()->clear();

	int const sel = availableLB->currentItem();
	if (sel < 0) {
		addPB->setEnabled(false);
		return;
	}

	addPB->setEnabled(true);
//	infoML->document()->setPlainText(form_->getKeyInfo(sel));
}


void QCitationFind::on_availableLB_selected(Q3ListBoxItem *)
{
	int const sel = availableLB->currentItem();
	foundkeys.clear();
//	foundkeys.push_back(form_->availableKeys()[sel]);
	emit newCitations();
	accept();
}

void QCitationFind::on_addPB_clicked()
{
//	form_->addKeys(availableLB->selectionModel()->selectedIndexes());

	int const sel = availableLB->currentItem();

	if (sel < 0)
		return;

	QStringList bibkeys = form_->available()->stringList();

	// Add the selected browser_bib keys to browser_cite
	// multiple selections are possible
	for (unsigned int i = 0; i != availableLB->count(); i++) {
		if (availableLB->isSelected(i)) {
				foundkeys.push_back(fromqstr(bibkeys[i]));
		}
	}

	emit newCitations();
	accept();
}


void QCitationFind::previous()
{
	find(biblio::BACKWARD);
}


void QCitationFind::next()
{
	find(biblio::FORWARD);
}


void QCitationFind::find(biblio::Direction dir)
{
/*	QStringList bibkeys = form_->available()->stringList();

	biblio::InfoMap const & theMap = form_->controller().bibkeysInfo();

	biblio::Search const type = searchTypeCB->isChecked()
		? biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = bibkeys.begin();
	int const sel = availableLB->currentItem();
	if (sel >= 0 && sel <= int(bibkeys.size()-1))
		start += sel;

	// Find the NEXT instance...
	if (dir == biblio::FORWARD)
		start += 1;

	bool const casesens = searchCaseCB->isChecked();
	string const str = fromqstr(searchED->text());

	vector<string>::const_iterator cit =
		biblio::searchKeys(theMap, bibkeys, str,
				   start, type, dir, casesens);

	// not found. let's loop round
	if (cit == bibkeys.end()) {
		if (dir == biblio::FORWARD) {
			start = bibkeys.begin();
		}
		else start = bibkeys.end() - 1;

		cit = biblio::searchKeys(theMap, bibkeys, str,
					 start, type, dir, casesens);

		if (cit == bibkeys.end())
			return;
	}

	int const found = int(cit - bibkeys.begin());
	if (found == sel) {
		return;
	}

	// Update the display
	// note that we have multi selection mode!
	availableLB->setSelected(sel, false);
	availableLB->setSelected(found, true);
	availableLB->setCurrentItem(found);
	availableLB->ensureCurrentVisible();
*/
}

} // namespace frontend
} // namespace lyx
