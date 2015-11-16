/**
 * \file GuiBibtex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voß
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBibtex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "LyXRC.h"
#include "qt_helpers.h"
#include "Validator.h"

#include "ui_BibtexAddUi.h"

#include "ButtonPolicy.h"

#include "frontends/alert.h"

#include "insets/InsetBibtex.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h" // changeExtension
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiBibtex::GuiBibtex(GuiView & lv)
	: GuiDialog(lv, "bibtex", qt_("BibTeX Bibliography")),
	  params_(insetCode("bibtex"))
{
	setupUi(this);

	QDialog::setModal(true);

	connect(okPB, SIGNAL(clicked()),
		this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
	connect(stylePB, SIGNAL(clicked()),
		this, SLOT(browsePressed()));
	connect(deletePB, SIGNAL(clicked()),
		this, SLOT(deletePressed()));
	connect(upPB, SIGNAL(clicked()),
		this, SLOT(upPressed()));
	connect(downPB, SIGNAL(clicked()),
		this, SLOT(downPressed()));
	connect(styleCB, SIGNAL(editTextChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(databaseLW, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
		this, SLOT(databaseChanged()));
	connect(bibtocCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(btPrintCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(addBibPB, SIGNAL(clicked()),
		this, SLOT(addPressed()));
	connect(rescanPB, SIGNAL(clicked()),
		this, SLOT(rescanClicked()));

	add_ = new GuiBibtexAddDialog(this);
	add_bc_.setPolicy(ButtonPolicy::OkCancelPolicy);
	add_bc_.setOK(add_->addPB);
	add_bc_.setCancel(add_->closePB);
	add_bc_.addCheckedLineEdit(add_->bibED, 0);

	connect(add_->bibED, SIGNAL(textChanged(QString)),
		this, SLOT(bibEDChanged()));
	connect(add_->addPB, SIGNAL(clicked()),
		this, SLOT(addDatabase()));
	connect(add_->addPB, SIGNAL(clicked()),
		add_, SLOT(accept()));
	connect(add_->rescanPB, SIGNAL(clicked()),
		this, SLOT(rescanClicked()));
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(addDatabase()));
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		add_, SLOT(accept()));
	connect(add_->bibLW, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
		this, SLOT(availableChanged()));
	connect(add_->browsePB, SIGNAL(clicked()),
		this, SLOT(browseBibPressed()));
	connect(add_->closePB, SIGNAL(clicked()),
		add_, SLOT(reject()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
	bc().addReadOnly(databaseLW);
	bc().addReadOnly(stylePB);
	bc().addReadOnly(styleCB);
	bc().addReadOnly(bibtocCB);
	bc().addReadOnly(addBibPB);
	// Delete/Up/Down are handled with more conditions in
	// databaseChanged().

	// Make sure the delete/up/down buttons are disabled if necessary.
	databaseChanged();
}


void GuiBibtex::bibEDChanged()
{
	// Indicate to the button controller that the contents have
	// changed. The actual test of validity is carried out by
	// the checkedLineEdit.
	add_bc_.setValid(true);
}


void GuiBibtex::change_adaptor()
{
	changed();
}


void GuiBibtex::browsePressed()
{
	QString const file = browseBst(QString());

	if (file.isEmpty())
		return;

	QString const filen = changeExtension(file, "");
	bool present = false;
	unsigned int pres = 0;

	for (int i = 0; i != styleCB->count(); ++i) {
		if (styleCB->itemText(i) == filen) {
			present = true;
			pres = i;
		}
	}

	if (!present)
		styleCB->insertItem(0, filen);

	styleCB->setCurrentIndex(pres);
	changed();
}


void GuiBibtex::browseBibPressed()
{
	QString const file = browseBib(QString()).trimmed();

	if (file.isEmpty())
		return;

	QString const f = changeExtension(file, "");
	bool present = false;

	for (int i = 0; i < add_->bibLW->count(); ++i) {
		if (add_->bibLW->item(i)->text() == f)
			present = true;
	}

	if (!present) {
		add_->bibLW->addItem(f);
		changed();
	}

	add_->bibED->setText(f);
}


void GuiBibtex::addPressed()
{
	add_bc_.setValid(false);
	add_->exec();
}


void GuiBibtex::addDatabase()
{
	int const sel = add_->bibLW->currentRow();
	QString const file = add_->bibED->text().trimmed();

	if (sel < 0 && file.isEmpty())
		return;

	// Add the selected browser_bib keys to browser_database
	// multiple selections are possible
	for (int i = 0; i != add_->bibLW->count(); ++i) {
		QListWidgetItem * const item = add_->bibLW->item(i);
		if (add_->bibLW->isItemSelected(item)) {
			add_->bibLW->setItemSelected(item, false);
			QList<QListWidgetItem *> matches =
				databaseLW->findItems(item->text(), Qt::MatchExactly);
			if (matches.empty()) {
				QString label = item->text();
				QListWidgetItem * db = new QListWidgetItem(label);
				db->setFlags(db->flags() | Qt::ItemIsSelectable);
				databaseLW->addItem(db);
			}
		}
	}

	if (!file.isEmpty()) {
		add_->bibED->clear();
		QString const f = changeExtension(file, "");
		QList<QListWidgetItem *> matches =
			databaseLW->findItems(f, Qt::MatchExactly);
		if (matches.empty()) {
			QListWidgetItem * db = new QListWidgetItem(f);
			db->setFlags(db->flags() | Qt::ItemIsSelectable);
			databaseLW->addItem(db);
		}
	}

	databaseChanged();
	changed();
}


void GuiBibtex::deletePressed()
{
	QListWidgetItem *cur = databaseLW->takeItem(databaseLW->currentRow());
	if (cur) {
		delete cur;
		databaseChanged();
		changed();
	}
}


void GuiBibtex::upPressed()
{
	int row = databaseLW->currentRow();
	QListWidgetItem *cur = databaseLW->takeItem(row);
	databaseLW->insertItem(row - 1, cur);
	databaseLW->setCurrentItem(cur);
	changed();
}


void GuiBibtex::downPressed()
{
	int row = databaseLW->currentRow();
	QListWidgetItem *cur = databaseLW->takeItem(row);
	databaseLW->insertItem(row + 1, cur);
	databaseLW->setCurrentItem(cur);
	changed();
}


void GuiBibtex::rescanClicked()
{
	rescanBibStyles();
	updateContents();
}


void GuiBibtex::databaseChanged()
{
	bool readOnly = isBufferReadonly();
	int count = databaseLW->count();
	int row = databaseLW->currentRow();
	deletePB->setEnabled(!readOnly && row != -1);
	upPB->setEnabled(!readOnly && count > 1 && row > 0);
	downPB->setEnabled(!readOnly && count > 1 && row < count - 1);
}


void GuiBibtex::availableChanged()
{
	add_bc_.setValid(true);
}


void GuiBibtex::updateContents()
{
	bool bibtopic = usingBibtopic();

	databaseLW->clear();

	docstring bibs = params_["bibfiles"];
	docstring bib;

	while (!bibs.empty()) {
		bibs = split(bibs, bib, ',');
		bib = trim(bib);
		if (!bib.empty()) {
			QListWidgetItem * db = new QListWidgetItem(toqstr(bib));
			db->setFlags(db->flags() | Qt::ItemIsSelectable);
			databaseLW->addItem(db);
		}
	}

	add_->bibLW->clear();

	QStringList bibfiles = bibFiles();
	for (int i = 0; i != bibfiles.count(); ++i)
		add_->bibLW->addItem(changeExtension(bibfiles[i], ""));

	QString const bibstyle = styleFile();

	bibtocCB->setChecked(bibtotoc() && !bibtopic);
	bibtocCB->setEnabled(!bibtopic);

	if (!bibtopic && btPrintCO->count() == 3)
		btPrintCO->removeItem(1);
	else if (bibtopic && btPrintCO->count() < 3)
		btPrintCO->insertItem(1, qt_("all uncited references", 0));

	docstring const & btprint = params_["btprint"];
	int btp = 0;
	if ((bibtopic && btprint == from_ascii("btPrintNotCited")) ||
	   (!bibtopic && btprint == from_ascii("btPrintAll")))
		btp = 1;
	else if (bibtopic && btprint == from_ascii("btPrintAll"))
		btp = 2;

	btPrintCO->setCurrentIndex(btp);

	styleCB->clear();

	int item_nr = -1;

	QStringList const str = bibStyles();
	for (int i = 0; i != str.count(); ++i) {
		QString item = changeExtension(str[i], "");
		if (item == bibstyle)
			item_nr = i;
		styleCB->addItem(item);
	}

	if (item_nr == -1 && !bibstyle.isEmpty()) {
		styleCB->addItem(bibstyle);
		item_nr = styleCB->count() - 1;
	}

	if (item_nr != -1)
		styleCB->setCurrentIndex(item_nr);
	else
		styleCB->clearEditText();
}


void GuiBibtex::applyView()
{
	docstring dbs;

	unsigned int maxCount = databaseLW->count();
	for (unsigned int i = 0; i < maxCount; i++) {
		if (i != 0)
			dbs += ',';
		QString item = databaseLW->item(i)->text();
		docstring bibfile = qstring_to_ucs4(item);
		dbs += bibfile;
	}

	params_["bibfiles"] = dbs;

	docstring const bibstyle = qstring_to_ucs4(styleCB->currentText());
	bool const bibtotoc = bibtocCB->isChecked();

	if (bibtotoc && !bibstyle.empty()) {
		// both bibtotoc and style
		params_["options"] = "bibtotoc," + bibstyle;
	} else if (bibtotoc) {
		// bibtotoc and no style
		params_["options"] = from_ascii("bibtotoc");
	} else {
		// only style. An empty one is valid, because some
		// documentclasses have an own \bibliographystyle{}
		// command!
		params_["options"] = bibstyle;
	}

	int btp = btPrintCO->currentIndex();

	if (usingBibtopic()) {
		// bibtopic allows three kinds of sections:
		// 1. sections that include all cited references of the database(s)
		// 2. sections that include all uncited references of the database(s)
		// 3. sections that include all references of the database(s), cited or not
		switch (btp) {
		case 0:
			params_["btprint"] = from_ascii("btPrintCited");
			break;
		case 1:
			params_["btprint"] = from_ascii("btPrintNotCited");
			break;
		case 2:
			params_["btprint"] = from_ascii("btPrintAll");
			break;
		}
	} else {
		switch (btp) {
		case 0:
			params_["btprint"] = docstring();
			break;
		case 1:
			// use \nocite{*}
			params_["btprint"] = from_ascii("btPrintAll");
			break;
		}
	}
}


bool GuiBibtex::isValid()
{
	return databaseLW->count() != 0;
}


QString GuiBibtex::browseBib(QString const & in_name) const
{
	QString const label1 = qt_("Documents|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);
	QStringList const filter(qt_("BibTeX Databases (*.bib)"));
	return browseRelToParent(in_name, bufferFilePath(),
		qt_("Select a BibTeX database to add"), filter, false, label1, dir1);
}


QString GuiBibtex::browseBst(QString const & in_name) const
{
	QString const label1 = qt_("Documents|#o#O");
	QString const dir1 = toqstr(lyxrc.document_path);
	QStringList const filter(qt_("BibTeX Styles (*.bst)"));
	return browseRelToParent(in_name, bufferFilePath(),
		qt_("Select a BibTeX style"), filter, false, label1, dir1);
}


QStringList GuiBibtex::bibStyles() const
{
	QStringList data = texFileList("bstFiles.lst");
	// test whether we have a valid list, otherwise run rescan
	if (data.isEmpty()) {
		rescanBibStyles();
		data = texFileList("bstFiles.lst");
	}
	for (int i = 0; i != data.size(); ++i)
		data[i] = onlyFileName(data[i]);
	// sort on filename only (no path)
	data.sort();
	return data;
}


QStringList GuiBibtex::bibFiles() const
{
	QStringList data = texFileList("bibFiles.lst");
	// test whether we have a valid list, otherwise run rescan
	if (data.isEmpty()) {
		rescanBibStyles();
		data = texFileList("bibFiles.lst");
	}
	for (int i = 0; i != data.size(); ++i)
		data[i] = onlyFileName(data[i]);
	// sort on filename only (no path)
	data.sort();
	return data;
}


void GuiBibtex::rescanBibStyles() const
{
	rescanTexStyles("bst bib");
}


bool GuiBibtex::usingBibtopic() const
{
	return buffer().params().use_bibtopic;
}


bool GuiBibtex::bibtotoc() const
{
	return prefixIs(to_utf8(params_["options"]), "bibtotoc");
}


QString GuiBibtex::styleFile() const
{
	// the different bibtex packages have (and need) their
	// own "plain" stylefiles
	QString defaultstyle = toqstr(buffer().params().defaultBiblioStyle());

	QString bst = toqstr(params_["options"]);
	if (bibtotoc()){
		// bibstyle exists?
		int pos = bst.indexOf(',');
		if (pos != -1) {
			// FIXME: check
			// docstring bibtotoc = from_ascii("bibtotoc");
			// bst = split(bst, bibtotoc, ',');
			bst = bst.mid(pos + 1);
		} else {
			bst.clear();
		}
	}

	// propose default style file for new insets
	// existing insets might have (legally) no bst files
	// (if the class already provides a style)
	if (bst.isEmpty() && params_["bibfiles"].empty())
		bst = defaultstyle;

	return bst;
}


bool GuiBibtex::initialiseParams(std::string const & data)
{
	InsetCommand::string2params(data, params_);
	return true;
}


void GuiBibtex::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}



Dialog * createGuiBibtex(GuiView & lv) { return new GuiBibtex(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiBibtex.cpp"
