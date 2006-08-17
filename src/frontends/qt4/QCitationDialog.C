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
#include "QCitation.h"
#include "qt_helpers.h"

#include "bufferparams.h"

#include "controllers/ControlCitation.h"

#include "support/lstrings.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace lyx {

using support::getStringFromVector;
using support::getVectorFromString;

namespace frontend {


QCitationDialog::QCitationDialog(Dialog & dialog, QCitation * form)
	: Dialog::View(dialog, "Citation"), form_(form)
{
	setupUi(this);

	setWindowTitle(toqstr("LyX: " + getTitle()));

 	selectedLV->setModel(form_->selected());
	availableLV->setModel(form_->available());

    connect(citationStyleCO, SIGNAL(activated(int)),
		this, SLOT(changed()));
    connect(fulllistCB, SIGNAL(clicked()),
		this, SLOT(changed()));
    connect(forceuppercaseCB, SIGNAL(clicked()),
		this, SLOT(changed()));
    connect(textBeforeED, SIGNAL(textChanged(const QString&)),
		this, SLOT(changed()));
    connect(textAfterED, SIGNAL(textChanged(const QString&)),
		this, SLOT(changed()));
}


QCitationDialog::~QCitationDialog()
{
}


void QCitationDialog::apply()
{
	int  const choice = std::max(0, citationStyleCO->currentIndex());
	style_ = choice;
	bool const full  = fulllistCB->isChecked();
	bool const force = forceuppercaseCB->isChecked();

	QString const before = textBeforeED->text();
	QString const after = textAfterED->text();

	form_->apply(choice, full, force, before, after);
}


void QCitationDialog::hide()
{
	accept();
}


void QCitationDialog::show()
{
	QDialog::show();
}


bool QCitationDialog::isVisible() const
{
	return QDialog::isVisible();
}
 

void QCitationDialog::on_okPB_clicked()
{
	apply();
	accept();
}


void QCitationDialog::on_cancelPB_clicked()
{
	accept();
}


void QCitationDialog::on_applyPB_clicked()
{
	apply();
}


void QCitationDialog::on_restorePB_clicked()
{
	update();
}


void QCitationDialog::update()
{
	form_->updateModel();

	QModelIndex idxa = availableLV->currentIndex();
	if (!idxa.isValid())
		availableLV->setCurrentIndex(availableLV->model()->index(0,0));

	QModelIndex idx = selectedLV->currentIndex();
	if (form_->isValid() && !idx.isValid()) {
		selectedLV->setCurrentIndex(selectedLV->model()->index(0,0));
		updateInfo(selectedLV->currentIndex());
	} else
		updateInfo(availableLV->currentIndex());

	setButtons();

	textBeforeED->setText(form_->textBefore());
	textAfterED->setText(form_->textAfter());

	fillStyles();
	updateStyle();
}


void QCitationDialog::updateStyle()
{
	biblio::CiteEngine const engine = form_->getEngine();
	bool const natbib_engine =
		engine == biblio::ENGINE_NATBIB_AUTHORYEAR ||
		engine == biblio::ENGINE_NATBIB_NUMERICAL;
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	fulllistCB->setEnabled(natbib_engine);
	forceuppercaseCB->setEnabled(natbib_engine);
	textBeforeED->setEnabled(!basic_engine);

	string const & command = form_->params().getCmdName();

	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();
	biblio::CitationStyle const cs(command);

	vector<biblio::CiteStyle>::const_iterator cit =
		std::find(styles.begin(), styles.end(), cs.style);

	// restore the latest natbib style
	if (style_ >= 0 && style_ < citationStyleCO->count())
		citationStyleCO->setCurrentIndex(style_);
	else
		citationStyleCO->setCurrentIndex(0);

	fulllistCB->setChecked(false);
	forceuppercaseCB->setChecked(false);

	if (cit != styles.end()) {
		int const i = int(cit - styles.begin());
		citationStyleCO->setCurrentIndex(i);
		fulllistCB->setChecked(cs.full);
		forceuppercaseCB->setChecked(cs.forceUCase);
	}
}


void QCitationDialog::fillStyles()
{
	int const orig = citationStyleCO->currentIndex();

	citationStyleCO->clear();

	QStringList selected_keys = form_->selected()->stringList();
	if (selected_keys.empty()) {
		citationStyleCO->setEnabled(false);
		citationStyleLA->setEnabled(false);
		return;
	}

	if (selectedLV->selectionModel()->selectedIndexes().empty())
		return;
	
	int curr = selectedLV->selectionModel()->selectedIndexes()[0].row();//selectedLV->currentItem();

	QStringList sty = form_->citationStyles(curr);

	bool const basic_engine = 
		(form_->getEngine() == biblio::ENGINE_BASIC);

	citationStyleCO->setEnabled(!sty.isEmpty() && !basic_engine);
	citationStyleLA->setEnabled(!sty.isEmpty() && !basic_engine);

	citationStyleCO->insertItems(0, sty);

	if (orig != -1 && orig < citationStyleCO->count())
		citationStyleCO->setCurrentIndex(orig);
}


bool QCitationDialog::isSelected(const QModelIndex & idx)
{
	QString const str = idx.data().toString();
	return !form_->selected()->stringList().filter(str).isEmpty();
}


void QCitationDialog::setButtons()
{
	int const arows = availableLV->model()->rowCount();
	addPB->setEnabled(arows>0 && !isSelected(availableLV->currentIndex()));

	int const srows = selectedLV->model()->rowCount();
	int const sel_nr = selectedLV->currentIndex().row();
	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < srows - 1);
	applyPB->setEnabled(srows>0);
	okPB->setEnabled(srows>0);
}


void QCitationDialog::updateInfo(const QModelIndex & idx)
{
	if (idx.isValid()) {
		QString const keytxt = form_->getKeyInfo(idx.data().toString());
		infoML->document()->setPlainText(keytxt);
	} else
		infoML->document()->clear();
}


void QCitationDialog::on_selectedLV_clicked(const QModelIndex & idx)
{
	updateInfo(idx);
	changed();
}

void QCitationDialog::on_availableLV_clicked(const QModelIndex & idx)
{
	updateInfo(idx);
	setButtons();
}


void QCitationDialog::on_availableLV_activated(const QModelIndex & idx)
{
	if (isSelected(idx))
		return;

	on_addPB_clicked();		
}


void QCitationDialog::on_addPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	form_->addKey(availableLV->currentIndex());
	if (idx.isValid())
		selectedLV->setCurrentIndex(idx);
	changed();
}


void QCitationDialog::on_deletePB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	int nrows = selectedLV->model()->rowCount();
	
	form_->deleteKey(idx);

	if (idx.row() == nrows - 1)	
		idx = idx.sibling(idx.row() - 1, idx.column());

	if (nrows>1)
		selectedLV->setCurrentIndex(idx);

	updateInfo(selectedLV->currentIndex());
	changed();
}


void QCitationDialog::on_upPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	form_->upKey(idx);
	selectedLV->setCurrentIndex(idx.sibling(idx.row() - 1, idx.column()));
	changed();
}


void QCitationDialog::on_downPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	form_->downKey(idx);
	selectedLV->setCurrentIndex(idx.sibling(idx.row() + 1, idx.column()));
	changed();
}


void QCitationDialog::on_findLE_textChanged(const QString & text)
{
	form_->findKey(text);
	availableLV->setModel(form_->found());
	changed();
}


void QCitationDialog::changed()
{
	fillStyles();
	setButtons();
}


} // namespace frontend
} // namespace lyx

#include "QCitationDialog_moc.cpp"
