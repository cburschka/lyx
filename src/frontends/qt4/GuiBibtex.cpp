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
#include "debug.h"
#include "ui_BibtexAddUi.h"
#include "qt_helpers.h"
#include "Validator.h"
#include "LyXRC.h"
#include "gettext.h"

#include "ButtonPolicy.h"

#include "support/filetools.h" // changeExtension
#include "support/lstrings.h"
#include "support/FileFilterList.h"

#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QCloseEvent>
#include <QLineEdit>

using std::pair;
using std::string;
using std::vector;


namespace lyx {
namespace frontend {

using support::changeExtension;
using support::contains;
using support::FileFilterList;
using support::onlyFilename;
using support::prefixIs;
using support::split;
using support::trim;


GuiBibtex::GuiBibtex(GuiView & lv)
	: GuiCommand(lv, "bibtex")
{
	setupUi(this);

	setViewTitle( _("BibTeX Bibliography"));

	QDialog::setModal(true);

	connect(okPB, SIGNAL(clicked()),
		this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
	connect(stylePB, SIGNAL(clicked()),
		this, SLOT(browsePressed()));
	connect(deletePB, SIGNAL(clicked()),
		this, SLOT(deletePressed()));
	connect(styleCB, SIGNAL(editTextChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(databaseLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(databaseChanged()));
	connect(bibtocCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(btPrintCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(addBibPB, SIGNAL(clicked()),
		this, SLOT(addPressed()));

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
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		this, SLOT(addDatabase()));
	connect(add_->bibLW, SIGNAL(itemActivated(QListWidgetItem *)),
		add_, SLOT(accept()));
	connect(add_->bibLW, SIGNAL(itemSelectionChanged()),
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
	bc().addReadOnly(deletePB);
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
	docstring const file = browseBst(docstring());

	if (!file.empty()) {
		// FIXME UNICODE
		docstring const filen = from_utf8(changeExtension(to_utf8(file), ""));
		bool present = false;
		unsigned int pres = 0;

		for (int i = 0; i != styleCB->count(); ++i) {
			if (qstring_to_ucs4(styleCB->itemText(i)) == filen) {
				present = true;
				pres = i;
			}
		}

		if (!present)
			styleCB->insertItem(0, toqstr(filen));

		styleCB->setCurrentIndex(pres);
		changed();
	}
}


void GuiBibtex::browseBibPressed()
{
	docstring const file = trim(browseBib(docstring()));

	if (!file.empty()) {
		// FIXME UNICODE
		QString const f = toqstr(changeExtension(to_utf8(file), ""));
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
}


void GuiBibtex::addPressed()
{
	add_bc_.setValid(false);
	add_->exec();
}


void GuiBibtex::addDatabase()
{
	int const sel = add_->bibLW->currentRow();
	docstring const file = trim(qstring_to_ucs4(add_->bibED->text()));

	if (sel < 0 && file.empty())
		return;

	// Add the selected browser_bib keys to browser_database
	// multiple selections are possible
	for (int i = 0; i != add_->bibLW->count(); ++i) {
		QListWidgetItem * const item = add_->bibLW->item(i);
		if (add_->bibLW->isItemSelected(item)) {
			add_->bibLW->setItemSelected(item, false);
			QList<QListWidgetItem *> matches =
				databaseLW->findItems(item->text(), Qt::MatchExactly);
			if (matches.empty())
				databaseLW->addItem(item->text());
		}
	}

	if (!file.empty()) {
		add_->bibED->clear();
		QString const f = toqstr(from_utf8(changeExtension(to_utf8(file), "")));
		QList<QListWidgetItem *> matches =
			databaseLW->findItems(f, Qt::MatchExactly);
		if (matches.empty())
			databaseLW->addItem(f);
	}

	changed();
}


void GuiBibtex::deletePressed()
{
	databaseLW->takeItem(databaseLW->currentRow());
	changed();
}



void GuiBibtex::databaseChanged()
{
	deletePB->setEnabled(!isBufferReadonly() && databaseLW->currentRow() != -1);
}


void GuiBibtex::availableChanged()
{
	add_bc_.setValid(true);
}


void GuiBibtex::closeEvent(QCloseEvent *e)
{
	slotClose();
	e->accept();
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
		if (!bib.empty())
			databaseLW->addItem(toqstr(bib));
	}

	add_->bibLW->clear();

	vector<string> bib_str;
	getBibFiles(bib_str);
	for (vector<string>::const_iterator it = bib_str.begin();
		it != bib_str.end(); ++it) {
		string bibItem(changeExtension(*it, ""));
		add_->bibLW->addItem(toqstr(bibItem));
	}

	string bibstyle = getStylefile();

	bibtocCB->setChecked(bibtotoc() && !bibtopic);
	bibtocCB->setEnabled(!bibtopic);

	docstring btprint = params_["btprint"];
	int btp = 0;
	if (btprint == "btPrintNotCited")
		btp = 1;
	else if (btprint == "btPrintAll")
		btp = 2;

	btPrintCO->setCurrentIndex(btp);
	btPrintCO->setEnabled(bibtopic);

	styleCB->clear();

	int item_nr(-1);

	vector<string> str;
	getBibStyles(str);
	for (vector<string>::const_iterator it = str.begin();
		it != str.end(); ++it) {
		string item(changeExtension(*it, ""));
		if (item == bibstyle)
			item_nr = int(it - str.begin());
		styleCB->addItem(toqstr(item));
	}

	if (item_nr == -1 && !bibstyle.empty()) {
		styleCB->addItem(toqstr(bibstyle));
		item_nr = styleCB->count() - 1;
	}

	if (item_nr != -1)
		styleCB->setCurrentIndex(item_nr);
	else
		styleCB->clearEditText();
}


void GuiBibtex::applyView()
{
	docstring dbs = qstring_to_ucs4(databaseLW->item(0)->text());

	unsigned int maxCount = databaseLW->count();
	for (unsigned int i = 1; i < maxCount; i++) {
		dbs += ',';
		dbs += qstring_to_ucs4(databaseLW->item(i)->text());
	}

	params_["bibfiles"] = dbs;

	docstring const bibstyle = qstring_to_ucs4(styleCB->currentText());
	bool const bibtotoc = bibtocCB->isChecked();

	if (bibtotoc && (!bibstyle.empty())) {
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

	// bibtopic allows three kinds of sections:
	// 1. sections that include all cited references of the database(s)
	// 2. sections that include all uncited references of the database(s)
	// 3. sections that include all references of the database(s), cited or not
	int btp = btPrintCO->currentIndex();

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

	if (!usingBibtopic())
		params_["btprint"] = docstring();
}


bool GuiBibtex::isValid()
{
	return databaseLW->count() != 0;
}


docstring const GuiBibtex::browseBib(docstring const & in_name) const
{
	// FIXME UNICODE
	docstring const label1 = _("Documents|#o#O");
	docstring const dir1 = from_utf8(lyxrc.document_path);
	FileFilterList const filter(_("BibTeX Databases (*.bib)"));
	return browseRelFile(in_name, from_utf8(bufferFilepath()),
		_("Select a BibTeX database to add"), filter, false, label1, dir1);
}


docstring const GuiBibtex::browseBst(docstring const & in_name) const
{
	// FIXME UNICODE
	docstring const label1 = _("Documents|#o#O");
	docstring const dir1 = from_utf8(lyxrc.document_path);
	FileFilterList const filter(_("BibTeX Styles (*.bst)"));
	return browseRelFile(in_name, from_utf8(bufferFilepath()),
		_("Select a BibTeX style"), filter, false, label1, dir1);
}


void GuiBibtex::getBibStyles(vector<string> & data) const
{
	data.clear();

	getTexFileList("bstFiles.lst", data);
	// test, if we have a valid list, otherwise run rescan
	if (data.empty()) {
		rescanBibStyles();
		getTexFileList("bstFiles.lst", data);
	}
	vector<string>::iterator it  = data.begin();
	vector<string>::iterator end = data.end();
	for (; it != end; ++it) {
		*it = onlyFilename(*it);
	}
	// sort on filename only (no path)
	std::sort(data.begin(), data.end());
}


void GuiBibtex::getBibFiles(vector<string> & data) const
{
	data.clear();

	getTexFileList("bibFiles.lst", data);
	// test, if we have a valid list, otherwise run rescan
	if (data.empty()) {
		rescanBibStyles();
		getTexFileList("bibFiles.lst", data);
	}
	vector<string>::iterator it  = data.begin();
	vector<string>::iterator end = data.end();
	for (; it != end; ++it) {
		*it = onlyFilename(*it);
	}
	// sort on filename only (no path)
	std::sort(data.begin(), data.end());
}


void GuiBibtex::rescanBibStyles() const
{
	rescanTexStyles();
}


bool GuiBibtex::usingBibtopic() const
{
	return buffer().params().use_bibtopic;
}


bool GuiBibtex::bibtotoc() const
{
	return prefixIs(to_utf8(params_["options"]), "bibtotoc");
}


string const GuiBibtex::getStylefile() const
{
	// the different bibtex packages have (and need) their
	// own "plain" stylefiles
	biblio::CiteEngine const engine = buffer().params().getEngine();
	docstring defaultstyle;
	switch (engine) {
	case biblio::ENGINE_BASIC:
		defaultstyle = from_ascii("plain");
		break;
	case biblio::ENGINE_NATBIB_AUTHORYEAR:
		defaultstyle = from_ascii("plainnat");
		break;
	case biblio::ENGINE_NATBIB_NUMERICAL:
		defaultstyle = from_ascii("plainnat");
		break;
	case biblio::ENGINE_JURABIB:
		defaultstyle = from_ascii("jurabib");
		break;
	}

	docstring bst = params_["options"];
	if (bibtotoc()){
		// bibstyle exists?
		if (contains(bst, ',')) {
			docstring bibtotoc = from_ascii("bibtotoc");
			bst = split(bst, bibtotoc, ',');
		} else
			bst.erase();
	}

	// propose default style file for new insets
	// existing insets might have (legally) no bst files
	// (if the class already provides a style)
	if (bst.empty() && params_["bibfiles"].empty())
		bst = defaultstyle;

	// FIXME UNICODE
	return to_utf8(bst);
}


Dialog * createGuiBibtex(GuiView & lv) { return new GuiBibtex(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiBibtex_moc.cpp"
