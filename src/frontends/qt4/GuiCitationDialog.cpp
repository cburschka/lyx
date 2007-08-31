/**
 * \file GuiCitationDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author John Levon
 * \author Jürgen Spitzmüller
 * \author Abdelrazak Younes
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCitationDialog.h"

#include "GuiCitation.h"

#include "frontends/controllers/frontend_helpers.h"
#include "frontends/controllers/ControlCitation.h"
#include "qt_helpers.h"

#include "support/docstring.h"

#include "debug.h"
#include "gettext.h"

#include <algorithm>
#include <vector>
#include <string>

#include <QCloseEvent>
#include <QKeyEvent>

#undef KeyPress

using std::vector;
using std::string;

namespace lyx {
namespace frontend {


GuiCitationDialog::GuiCitationDialog(Dialog & dialog, GuiCitation * form)
	: Dialog::View(dialog, _("Citation")), form_(form)
{
	setupUi(this);

	setWindowTitle(toqstr("LyX: " + getTitle()));

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
	connect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));

	selectionManager = 
		new GuiSelectionManager(availableLV, selectedLV, 
		                      addPB, deletePB, upPB, downPB, 
		                      form_->available(), form_->selected());
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(setCitedKeys()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateDialog()));
	connect(selectionManager, SIGNAL(okHook()),
					this, SLOT(on_okPB_clicked()));

}


GuiCitationDialog::~GuiCitationDialog()
{}


void GuiCitationDialog::cleanUp() 
{
	form_->clearSelection();
	form_->clearParams();
	close();
}


void GuiCitationDialog::closeEvent(QCloseEvent * e)
{
	form_->clearSelection();
	form_->clearParams();
	e->accept();
}


void GuiCitationDialog::apply()
{
	int  const choice = std::max(0, citationStyleCO->currentIndex());
	style_ = choice;
	bool const full  = fulllistCB->isChecked();
	bool const force = forceuppercaseCB->isChecked();

	QString const before = textBeforeED->text();
	QString const after = textAfterED->text();

	form_->apply(choice, full, force, before, after);
}


void GuiCitationDialog::hide()
{
	form_->clearParams();
	accept();
}


void GuiCitationDialog::show()
{
	findLE->clear();
	availableLV->setFocus();
	QDialog::show();
	raise();
	activateWindow();
}


bool GuiCitationDialog::isVisible() const
{
	return QDialog::isVisible();
}


void GuiCitationDialog::on_okPB_clicked()
{
	apply();
	form_->clearSelection();
	hide();
}


void GuiCitationDialog::on_cancelPB_clicked()
{
	form_->clearSelection();
	hide();
}


void GuiCitationDialog::on_applyPB_clicked()
{
	apply();
}


void GuiCitationDialog::on_restorePB_clicked()
{
	form_->init();
	update();
}


void GuiCitationDialog::update()
{
	fillFields();
	fillEntries();
	updateDialog();
}


//The main point of separating this out is that the fill*() methods
//called in update() do not need to be called for INTERNAL updates,
//such as when addPB is pressed, as the list of fields, entries, etc,
//will not have changed. At the moment, however, the division between
//fillStyles() and updateStyles() doesn't lend itself to dividing the
//two methods, though they should be divisible.
void GuiCitationDialog::updateDialog()
{
	if (selectionManager->selectedFocused()) { 
		if (selectedLV->selectionModel()->selectedIndexes().isEmpty())
			updateInfo(availableLV->currentIndex());
		else
			updateInfo(selectedLV->currentIndex());
	} else {
		if (availableLV->selectionModel()->selectedIndexes().isEmpty())
			updateInfo(QModelIndex());
		else
			updateInfo(availableLV->currentIndex());
	}
	setButtons();

	textBeforeED->setText(form_->textBefore());
	textAfterED->setText(form_->textAfter());
	fillStyles();
	updateStyle();
}


void GuiCitationDialog::updateStyle()
{
	biblio::CiteEngine const engine = form_->getEngine();
	bool const natbib_engine =
		engine == biblio::ENGINE_NATBIB_AUTHORYEAR ||
		engine == biblio::ENGINE_NATBIB_NUMERICAL;
	bool const basic_engine = engine == biblio::ENGINE_BASIC;

	bool const haveSelection = 
		selectedLV->model()->rowCount() > 0;
	fulllistCB->setEnabled(natbib_engine && haveSelection);
	forceuppercaseCB->setEnabled(natbib_engine && haveSelection);
	textBeforeED->setEnabled(!basic_engine && haveSelection);
	textBeforeLA->setEnabled(!basic_engine && haveSelection);
	textAfterED->setEnabled(haveSelection);
	textAfterLA->setEnabled(haveSelection);
	citationStyleCO->setEnabled(!basic_engine && haveSelection);
	citationStyleLA->setEnabled(!basic_engine && haveSelection);

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

	if (cit != styles.end()) {
		int const i = int(cit - styles.begin());
		citationStyleCO->setCurrentIndex(i);
		fulllistCB->setChecked(cs.full);
		forceuppercaseCB->setChecked(cs.forceUCase);
	} else {
		fulllistCB->setChecked(false);
		forceuppercaseCB->setChecked(false);
	}
}


//This one needs to be called whenever citationStyleCO needs
//to be updated---and this would be on anything that changes the
//selection in selectedLV, or on a general update.
void GuiCitationDialog::fillStyles()
{
	int const oldIndex = citationStyleCO->currentIndex();

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

	if (oldIndex != -1 && oldIndex < citationStyleCO->count())
		citationStyleCO->setCurrentIndex(oldIndex);
}


void GuiCitationDialog::fillFields()
{
	fieldsCO->blockSignals(true);
	int const oldIndex = fieldsCO->currentIndex();
	fieldsCO->clear();
	QStringList const & fields = form_->getFieldsAsQStringList();
	fieldsCO->insertItem(0, qt_("All Fields"));
	fieldsCO->insertItem(1, qt_("Keys"));
	fieldsCO->insertItems(2, fields);
	if (oldIndex != -1 && oldIndex < fieldsCO->count())
		fieldsCO->setCurrentIndex(oldIndex);
	fieldsCO->blockSignals(false);
}


void GuiCitationDialog::fillEntries()
{
	entriesCO->blockSignals(true);
	int const oldIndex = entriesCO->currentIndex();
	entriesCO->clear();
	QStringList const & entries = form_->getEntriesAsQStringList();
	entriesCO->insertItem(0, qt_("All Entry Types"));
	entriesCO->insertItems(1, entries);
	if (oldIndex != -1 && oldIndex < entriesCO->count())
		entriesCO->setCurrentIndex(oldIndex);
	entriesCO->blockSignals(false);
}


bool GuiCitationDialog::isSelected(const QModelIndex & idx)
{
	QString const str = idx.data().toString();
	return form_->selected()->stringList().contains(str);
}


void GuiCitationDialog::setButtons()
{
	selectionManager->update();
	int const srows = selectedLV->model()->rowCount();
	applyPB->setEnabled(srows > 0);
	okPB->setEnabled(srows > 0);
}


void GuiCitationDialog::updateInfo(QModelIndex const & idx)
{
	if (idx.isValid()) {
		QString const keytxt = form_->getKeyInfo(idx.data().toString());
		infoML->document()->setPlainText(keytxt);
	} else
		infoML->document()->clear();
}


void GuiCitationDialog::setCitedKeys() 
{
	form_->setCitedKeys();
}


void GuiCitationDialog::findText(QString const & text, bool reset)
{
	//"All Fields" and "Keys" are the first two
	int index = fieldsCO->currentIndex() - 2; 
	vector<docstring> const & fields = form_->availableFields();
	docstring field;
	
	if (index <= -1 || index >= int(fields.size()))
		//either "All Fields" or "Keys" or an invalid value
		field = from_ascii("");
	else
		field = fields[index];
	
	//Was it "Keys"?
	bool const onlyKeys = index == -1;
	
	//"All Entry Types" is first.
	index = entriesCO->currentIndex() - 1; 
	vector<docstring> const & entries = form_->availableEntries();
	docstring entryType;
	if (index < 0 || index >= int(entries.size()))
		entryType = from_ascii("");
	else 
		entryType = entries[index];
	
	bool const case_sentitive = caseCB->checkState();
	bool const reg_exp = regexCB->checkState();
	form_->findKey(text, onlyKeys, field, entryType, 
	               case_sentitive, reg_exp, reset);
	//FIXME
	//It'd be nice to save and restore the current selection in 
	//availableLV. Currently, we get an automatic reset, since the
	//model is reset.
	
	updateDialog();
}


void GuiCitationDialog::on_fieldsCO_currentIndexChanged(int /*index*/)
{
	findText(findLE->text(), true);
}


void GuiCitationDialog::on_entriesCO_currentIndexChanged(int /*index*/)
{
	findText(findLE->text(), true);
}


void GuiCitationDialog::on_findLE_textChanged(const QString & text)
{
	clearPB->setDisabled(text.isEmpty());
	if (text.isEmpty())
		findLE->setFocus();
	findText(text);
}


void GuiCitationDialog::on_caseCB_stateChanged(int)
{
	findText(findLE->text());
}


void GuiCitationDialog::on_regexCB_stateChanged(int)
{
	findText(findLE->text());
}


void GuiCitationDialog::changed()
{
	fillStyles();
	setButtons();
}


} // namespace frontend
} // namespace lyx

#include "GuiCitationDialog_moc.cpp"
