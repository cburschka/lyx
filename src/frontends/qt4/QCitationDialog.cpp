/**
 * \file QCitationDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCitationDialog.h"

#include "QCitation.h"

#include "frontends/controllers/frontend_helpers.h"
#include "frontends/controllers/ControlCitation.h"

#include "debug.h"
#include "gettext.h"

#include <algorithm>
#include <vector>
#include <string>

#include <QCloseEvent>
#include <QKeyEvent>

using std::vector;
using std::string;

namespace lyx {
namespace frontend {


QCitationDialog::QCitationDialog(Dialog & dialog, QCitation * form)
	: Dialog::View(dialog, _("Citation")), form_(form)
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
	connect(clearPB, SIGNAL(clicked()),
		findLE, SLOT(clear()));
	connect(availableLV->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
		this, SLOT(availableChanged(const QModelIndex &, const QModelIndex &)));
	connect(selectedLV->selectionModel(),
		SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
		this, SLOT(selectedChanged(const QModelIndex &, const QModelIndex &)));
	connect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));
	availableLV->installEventFilter(this);
	selectedLV->installEventFilter(this);
}


QCitationDialog::~QCitationDialog()
{
}


bool QCitationDialog::eventFilter(QObject * obj, QEvent * event) 
{
	if (obj == availableLV) {
		if (event->type() != QEvent::KeyPress)
			return QObject::eventFilter(obj, event);
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		int const keyPressed = keyEvent->key();
		Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
		//Enter key without modifier will add current item.
		//Ctrl-Enter will add it and close the dialog.
		//This is designed to work both with the main enter key
		//and the one on the numeric keypad.
		if ((keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return) &&
				//We want one or both of Control and Keypad, and nothing else
				//(KeypadModifier is what you get if you use the Enter key on the
				//numeric keypad.)
				(!keyModifiers || 
				 (keyModifiers == Qt::ControlModifier) ||
				 (keyModifiers == Qt::KeypadModifier)  ||
				 (keyModifiers == (Qt::ControlModifier | Qt::KeypadModifier))
				)
			) {
				if (addPB->isEnabled())
					on_addPB_clicked();
				if (keyModifiers & Qt::ControlModifier)
					on_okPB_clicked();
				event->accept();
				return true;
		} 
	} else if (obj == selectedLV) {
		//Delete or backspace key will delete current item
		//...with control modifier will clear the list
		if (event->type() != QEvent::KeyPress)
			return QObject::eventFilter(obj, event);
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		int const keyPressed = keyEvent->key();
		Qt::KeyboardModifiers const keyModifiers = keyEvent->modifiers();
		if (keyPressed == Qt::Key_Delete || keyPressed == Qt::Key_Backspace) {
			if (keyModifiers == Qt::NoModifier && deletePB->isEnabled())
				on_deletePB_clicked();
			else if (keyModifiers == Qt::ControlModifier) {
				form_->clearSelection();
				update();
			} else
				//ignore it otherwise
				return QObject::eventFilter(obj, event);
			event->accept();
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}


void QCitationDialog::cleanUp() 
{
	form_->clearSelection();
	form_->clearParams();
	close();
}


void QCitationDialog::closeEvent(QCloseEvent * e)
{
	form_->clearSelection();
	form_->clearParams();
	e->accept();
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
	form_->clearParams();
	accept();
}


void QCitationDialog::show()
{
	findLE->clear();
	availableLV->setFocus();
	QDialog::show();
	raise();
	activateWindow();
}


bool QCitationDialog::isVisible() const
{
	return QDialog::isVisible();
}


void QCitationDialog::on_okPB_clicked()
{
	apply();
	form_->clearSelection();
	hide();
}


void QCitationDialog::on_cancelPB_clicked()
{
	form_->clearSelection();
	hide();
}


void QCitationDialog::on_applyPB_clicked()
{
	apply();
}


void QCitationDialog::on_restorePB_clicked()
{
	form_->init();
	update();
}


void QCitationDialog::update()
{
	if (selectedLV->selectionModel()->selectedIndexes().isEmpty()) {
		if (availableLV->selectionModel()->selectedIndexes().isEmpty()
			&& availableLV->model()->rowCount() > 0)
				availableLV->setCurrentIndex(availableLV->model()->index(0,0));
		updateInfo(availableLV->currentIndex());
	} else
		updateInfo(selectedLV->currentIndex());

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
	textBeforeLA->setEnabled(!basic_engine);

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

	int curr = selectedLV->model()->rowCount() - 1;
	if (curr < 0)
		return;

	if (!selectedLV->selectionModel()->selectedIndexes().empty())
		curr = selectedLV->selectionModel()->selectedIndexes()[0].row();

	QStringList sty = form_->citationStyles(curr);

	bool const basic_engine =
		(form_->getEngine() == biblio::ENGINE_BASIC);

	citationStyleCO->setEnabled(!sty.isEmpty() && !basic_engine);
	citationStyleLA->setEnabled(!sty.isEmpty() && !basic_engine);

	if (sty.isEmpty() || basic_engine)
		return;

	citationStyleCO->insertItems(0, sty);

	if (orig != -1 && orig < citationStyleCO->count())
		citationStyleCO->setCurrentIndex(orig);
}


bool QCitationDialog::isSelected(const QModelIndex & idx)
{
	QString const str = idx.data().toString();
	return form_->selected()->stringList().contains(str);
}


void QCitationDialog::setButtons()
{
	int const arows = availableLV->model()->rowCount();
	addPB->setEnabled(arows > 0 &&
		availableLV->currentIndex().isValid() &&
		!isSelected(availableLV->currentIndex()));

	int const srows = selectedLV->model()->rowCount();
	int const sel_nr = selectedLV->currentIndex().row();
	deletePB->setEnabled(sel_nr >= 0);
	upPB->setEnabled(sel_nr > 0);
	downPB->setEnabled(sel_nr >= 0 && sel_nr < srows - 1);
	applyPB->setEnabled(srows > 0);
	okPB->setEnabled(srows > 0);
}


void QCitationDialog::updateInfo(const QModelIndex & idx)
{
	if (idx.isValid()) {
		QString const keytxt = form_->getKeyInfo(idx.data().toString());
		infoML->document()->setPlainText(keytxt);
	} else
		infoML->document()->clear();
}


void QCitationDialog::on_selectedLV_clicked(const QModelIndex &)
{
	availableLV->selectionModel()->reset();
	update();
}


void QCitationDialog::selectedChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;

	availableLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_availableLV_clicked(const QModelIndex &)
{
	selectedLV->selectionModel()->reset();
	update();
}


void QCitationDialog::availableChanged(const QModelIndex & idx, const QModelIndex &)
{
	if (!idx.isValid())
		return;

	selectedLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_availableLV_doubleClicked(const QModelIndex & idx)
{
	if (isSelected(idx))
		return;

	selectedLV->selectionModel()->reset();
	on_addPB_clicked();
}


void QCitationDialog::on_availableLV_entered(const QModelIndex &)
{
}


namespace {
//helper function for next two
QModelIndex getSelectedIndex(QListView * lv) {
	//Encourage compiler to use NRVO
	QModelIndex retval = QModelIndex();
	QModelIndexList selIdx = 
		lv->selectionModel()->selectedIndexes();
	if (!selIdx.empty())
		retval = selIdx.first();
	return retval;
}
}//anonymous namespace


void QCitationDialog::on_addPB_clicked()
{
	QModelIndex const idxToAdd = getSelectedIndex(availableLV);
	if (!idxToAdd.isValid())
		return;
	QModelIndex idx = selectedLV->currentIndex();
	form_->addKey(idxToAdd);
	if (idx.isValid())
		selectedLV->setCurrentIndex(idx);
	selectedLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_deletePB_clicked()
{
	QModelIndex idx = getSelectedIndex(selectedLV);
	if (!idx.isValid())
		return;
	int nrows = selectedLV->model()->rowCount();

	form_->deleteKey(idx);

	if (idx.row() == nrows - 1)
		idx = idx.sibling(idx.row() - 1, idx.column());

	if (nrows>1)
		selectedLV->setCurrentIndex(idx);

	availableLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_upPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	form_->upKey(idx);
	selectedLV->setCurrentIndex(idx.sibling(idx.row() - 1, idx.column()));
	availableLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_downPB_clicked()
{
	QModelIndex idx = selectedLV->currentIndex();
	form_->downKey(idx);
	selectedLV->setCurrentIndex(idx.sibling(idx.row() + 1, idx.column()));
	availableLV->selectionModel()->reset();
	update();
}


void QCitationDialog::findText(QString const & text)
{
	bool const case_sentitive = caseCB->checkState();
	bool const reg_exp = regexCB->checkState();
	form_->findKey(text, false, case_sentitive, reg_exp);
	selectedLV->selectionModel()->reset();
	update();
}


void QCitationDialog::on_findLE_textChanged(const QString & text)
{
	clearPB->setDisabled(text.isEmpty());
	if (text.isEmpty())
		findLE->setFocus();
	findText(text);
}


void QCitationDialog::on_caseCB_stateChanged(int)
{
	findText(findLE->text());
}


void QCitationDialog::on_regexCB_stateChanged(int)
{
	findText(findLE->text());
}


void QCitationDialog::changed()
{
	fillStyles();
	setButtons();
}


} // namespace frontend
} // namespace lyx

#include "QCitationDialog_moc.cpp"
