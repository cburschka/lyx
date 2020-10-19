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
#include "CiteEnginesList.h"
#include "Encoding.h"
#include "FuncRequest.h"
#include "GuiApplication.h"
#include "LyXRC.h"
#include "qt_helpers.h"
#include "TextClass.h"
#include "Validator.h"

#include "ButtonPolicy.h"
#include "FancyLineEdit.h"

#include "frontends/alert.h"

#include "insets/InsetBibtex.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h" // changeExtension
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QDialogButtonBox>
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
	setWindowModality(Qt::WindowModal);

	// The filter bar
	filter_ = new FancyLineEdit(this);
	filter_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	filter_->setButtonVisible(FancyLineEdit::Right, true);
	filter_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	filter_->setAutoHideButton(FancyLineEdit::Right, true);
	filter_->setPlaceholderText(qt_("All avail. databases"));

	filterBarL->addWidget(filter_, 0);
	findKeysLA->setBuddy(filter_);

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));
	connect(stylePB, SIGNAL(clicked()),
		this, SLOT(browseBstPressed()));
	connect(styleCB, SIGNAL(editTextChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(bibtocCB, SIGNAL(clicked()),
		this, SLOT(change_adaptor()));
	connect(btPrintCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(rescanPB, SIGNAL(clicked()),
		this, SLOT(rescanClicked()));
	connect(biblatexOptsLE, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(bibEncodingCO, SIGNAL(activated(int)),
		this, SLOT(change_adaptor()));
	connect(browseBibPB, SIGNAL(clicked()),
		this, SLOT(browseBibPressed()));

	selected_model_.insertColumns(0, 1);
	selectionManager = new GuiSelectionManager(this, availableLV, selectedLV,
			addBibPB, deletePB, upPB, downPB, &available_model_, &selected_model_);
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(databaseChanged()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(selUpdated()));
	connect(selectionManager, SIGNAL(okHook()),
		this, SLOT(on_buttonBox_accepted()));

	connect(filter_, SIGNAL(rightButtonClicked()),
		this, SLOT(resetFilter()));
	connect(filter_, SIGNAL(textEdited(QString)),
		this, SLOT(filterChanged(QString)));
	connect(filter_, SIGNAL(returnPressed()),
		this, SLOT(filterPressed()));
#if (QT_VERSION < 0x050000)
	connect(filter_, SIGNAL(downPressed()),
	        availableLV, SLOT(setFocus()));
#else
	connect(filter_, &FancyLineEdit::downPressed,
	        availableLV, [=](){ focusAndHighlight(availableLV); });
#endif

	availableLV->setToolTip(formatToolTip(qt_("This list consists of all databases that are indexed by LaTeX and thus are found without a file path. "
				    "This is usually everything in the bib/ subdirectory of LaTeX's texmf tree. "
				    "If you want to reuse your own database, this is the place you should store it.")));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setApply(buttonBox->button(QDialogButtonBox::Apply));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
	bc().addReadOnly(stylePB);
	bc().addReadOnly(styleCB);
	bc().addReadOnly(bibtocCB);
	bc().addReadOnly(bibEncodingCO);

#if (QT_VERSION < 0x050000)
	selectedLV->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
	selectedLV->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif

	// Always put the default encoding in the first position.
	bibEncodingCO->addItem(qt_("Document Encoding"), "default");
	for (auto const & encvar : encodings) {
		if (!encvar.unsafe() && !encvar.guiName().empty())
			encodings_.insert(qt_(encvar.guiName()), toqstr(encvar.name()));
	}
	QMap<QString, QString>::const_iterator it = encodings_.constBegin();
	while (it != encodings_.constEnd()) {
		bibEncodingCO->addItem(it.key(), it.value());
		++it;
	}

	setFocusProxy(filter_);
}


void GuiBibtex::init()
{
	all_bibs_ = bibFiles(false);
	available_model_.setStringList(all_bibs_);

	QString bibs = toqstr(params_["bibfiles"]);
	if (bibs.isEmpty())
		selected_bibs_.clear();
	else
		selected_bibs_ = bibs.split(",");
	setSelectedBibs(selected_bibs_);

	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	selectionManager->update();
}


void GuiBibtex::change_adaptor()
{
	setButtons();
	changed();
}


void GuiBibtex::setButtons()
{
	int const srows = selectedLV->model()->rowCount();
	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(srows > 0);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(srows > 0);
}


void GuiBibtex::selUpdated()
{
	selectionManager->update();
	editPB->setEnabled(deletePB->isEnabled());
	changed();
}


void GuiBibtex::on_buttonBox_accepted()
{
	applyView();
	clearSelection();
	hide();
}


void GuiBibtex::browseBstPressed()
{
	QString const file = browseBst(QString());

	if (file.isEmpty())
		return;

	QString const filen = changeExtension(file, "");
	bool present = false;
	int pres = 0;

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

	if (!selected_bibs_.contains(f)) {
		selected_bibs_.append(f);
		setSelectedBibs(selected_bibs_);
		changed();
	}
}


void GuiBibtex::on_editPB_clicked()
{
	QModelIndexList selIdx =
		selectedLV->selectionModel()->selectedIndexes();
	if (selIdx.isEmpty())
		return;
	QModelIndex idx = selIdx.first();
	QString sel = idx.data().toString();
	FuncRequest fr(LFUN_INSET_EDIT, fromqstr(sel));
	dispatch(fr);
}


void GuiBibtex::rescanClicked()
{
	rescanBibStyles();
	updateContents();
}


void GuiBibtex::clearSelection()
{
	selected_bibs_.clear();
	setSelectedBibs(selected_bibs_);
}


void GuiBibtex::setSelectedBibs(QStringList const & sl)
{
	selected_model_.clear();
	QStringList headers;
	headers << qt_("Database")
		<< qt_("File Encoding");
	selected_model_.setHorizontalHeaderLabels(headers);
	bool const moreencs = usingBiblatex() && sl.count() > 1;
	selectedLV->setColumnHidden(1, !moreencs);
	selectedLV->verticalHeader()->setVisible(false);
	selectedLV->horizontalHeader()->setVisible(moreencs);
	if (moreencs) {
		bibEncodingLA->setText(qt_("General E&ncoding:"));
		bibEncodingCO->setToolTip(qt_("If your bibliography databases use a different "
					      "encoding than the LyX document, specify it here. "
					      "If indivivual databases have different encodings, "
					      "you can set it in the list above."));
	} else {
		bibEncodingLA->setText(qt_("E&ncoding:"));
		bibEncodingCO->setToolTip(qt_("If your bibliography databases use a different "
					      "encoding than the LyX document, specify it here"));
	}
	QStringList::const_iterator it  = sl.begin();
	QStringList::const_iterator end = sl.end();
	for (int i = 0; it != end; ++it, ++i) {
		QStandardItem * si = new QStandardItem();
		si->setData(*it);
		si->setText(*it);
		si->setToolTip(*it);
		si->setEditable(false);
		selected_model_.insertRow(i, si);
		QComboBox * cb = new QComboBox;
		cb->addItem(qt_("General Encoding"), "general");
		cb->addItem(qt_("Document Encoding"), "auto");
		QMap<QString, QString>::const_iterator it = encodings_.constBegin();
		while (it != encodings_.constEnd()) {
			cb->addItem(it.key(), it.value());
			++it;
		}
		cb->setToolTip(qt_("If this bibliography database uses a different "
				   "encoding than specified below, set it here"));
		selectedLV->setIndexWidget(selected_model_.index(i, 1), cb);
	}
	editPB->setEnabled(deletePB->isEnabled());
}


QStringList GuiBibtex::selectedBibs()
{
	QStringList res;
	for (int i = 0; i != selected_model_.rowCount(); ++i) {
		QStandardItem const * item = selected_model_.item(i);
		if (item)
			res.append(item->text());
	}
	return res;
}


void GuiBibtex::databaseChanged()
{
	selected_bibs_ = selectedBibs();
	setSelectedBibs(selected_bibs_);
}


void GuiBibtex::updateContents()
{
	bool const bibtopic = usingBibtopic();
	bool const biblatex = usingBiblatex();

	if (biblatex)
		setTitle(qt_("Biblatex Bibliography"));
	else
		setTitle(qt_("BibTeX Bibliography"));

	QString const bibstyle = styleFile();

	bool const hasbibintoc = buffer().params().documentClass().bibInToc()
			&& !biblatex;

	bibtocCB->setChecked((bibtotoc() && !bibtopic) || hasbibintoc);
	bibtocCB->setEnabled(!bibtopic && !hasbibintoc);

	btPrintCO->clear();
	btPrintCO->addItem(qt_("all cited references"), toqstr("btPrintCited"));
	if (bibtopic)
		btPrintCO->addItem(qt_("all uncited references"), toqstr("btPrintNotCited"));
	btPrintCO->addItem(qt_("all references"), toqstr("btPrintAll"));
	if (usingBiblatex() && !buffer().masterParams().multibib.empty())
		btPrintCO->addItem(qt_("all reference units"), toqstr("bibbysection"));

	docstring btprint = params_["btprint"];
	if (btprint.empty())
		// default
		btprint = from_ascii("btPrintCited");
	btPrintCO->setCurrentIndex(btPrintCO->findData(toqstr(btprint)));

	docstring encoding = params_["encoding"];
	if (encoding.empty())
		// default
		encoding = from_ascii("default");
	bibEncodingCO->setCurrentIndex(bibEncodingCO->findData(toqstr(encoding)));

	// Only useful for biblatex
	biblatexOptsLA->setVisible(biblatex);
	biblatexOptsLE->setVisible(biblatex);

	// only useful for BibTeX
	bstGB->setVisible(!biblatex);

	if (!biblatex) {
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
	} else
		biblatexOptsLE->setText(toqstr(params_["biblatexopts"]));

	setFileEncodings(getVectorFromString(params_["file_encodings"], from_ascii("\t")));
	editPB->setEnabled(deletePB->isEnabled());
}


void GuiBibtex::applyView()
{
	docstring dbs;

	int maxCount = selected_bibs_.count();
	for (int i = 0; i < maxCount; i++) {
		if (i != 0)
			dbs += ',';
		QString item = selected_bibs_.at(i);
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

	params_["biblatexopts"] = qstring_to_ucs4(biblatexOptsLE->text());

	params_["btprint"] = qstring_to_ucs4(btPrintCO->itemData(btPrintCO->currentIndex()).toString());

	params_["encoding"] = qstring_to_ucs4(bibEncodingCO->itemData(bibEncodingCO->currentIndex()).toString());

	if (usingBiblatex())
		params_["file_encodings"] = getStringFromVector(getFileEncodings(), from_ascii("\t"));
}


QString GuiBibtex::browseBib(QString const & in_name) const
{
	QString const label1 = qt_("D&ocuments");
	QString const dir1 = toqstr(lyxrc.document_path);
	QStringList const filter(qt_("BibTeX Databases (*.bib)"));
	return browseRelToParent(in_name, bufferFilePath(),
		qt_("Select a BibTeX database to add"), filter, false, label1, dir1);
}


QString GuiBibtex::browseBst(QString const & in_name) const
{
	QString const label1 = qt_("D&ocuments");
	QString const dir1 = toqstr(lyxrc.document_path);
	QStringList const filter(qt_("BibTeX Styles (*.bst)"));
	return browseRelToParent(in_name, bufferFilePath(),
		qt_("Select a BibTeX style"), filter, false, label1, dir1);
}


QStringList GuiBibtex::bibStyles() const
{
	QStringList sdata = texFileList("bstFiles.lst");
	// test whether we have a valid list, otherwise run rescan
	if (sdata.isEmpty()) {
		rescanBibStyles();
		sdata = texFileList("bstFiles.lst");
	}
	for (int i = 0; i != sdata.size(); ++i)
		sdata[i] = onlyFileName(sdata[i]);
	// sort on filename only (no path)
	sdata.sort();
	return sdata;
}


QStringList GuiBibtex::bibFiles(bool const extension) const
{
	QStringList sdata = texFileList("bibFiles.lst");
	// test whether we have a valid list, otherwise run rescan
	if (sdata.isEmpty()) {
		rescanBibStyles();
		sdata = texFileList("bibFiles.lst");
	}
	for (int i = 0; i != sdata.size(); ++i)
		sdata[i] = extension ? onlyFileName(sdata[i])
				     : changeExtension(onlyFileName(sdata[i]), "");
	// sort on filename only (no path)
	sdata.sort();
	return sdata;
}


vector<docstring> GuiBibtex::getFileEncodings()
{
	vector<docstring> res;
	for (int i = 0; i != selected_model_.rowCount(); ++i) {
		QStandardItem const * key = selected_model_.item(i, 0);
		QComboBox * cb = qobject_cast<QComboBox*>(selectedLV->indexWidget(selected_model_.index(i, 1)));
		QString fenc = cb ? cb->itemData(cb->currentIndex()).toString() : QString();
		if (key && !key->text().isEmpty() && !fenc.isEmpty() && fenc != "general")
			res.push_back(qstring_to_ucs4(key->text()) + " " + qstring_to_ucs4(fenc));
	}
	return res;
}


void GuiBibtex::setFileEncodings(vector<docstring> const & m)
{
	for (docstring const & s: m) {
		docstring key;
		QString enc = toqstr(split(s, key, ' '));
		QModelIndexList qmil =
				selected_model_.match(selected_model_.index(0, 0),
						     Qt::DisplayRole, toqstr(key), 1,
						     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
		if (!qmil.empty()) {
			QComboBox * cb = qobject_cast<QComboBox*>(selectedLV->indexWidget(selected_model_.index(qmil.front().row(), 1)));
			cb->setCurrentIndex(cb->findData(enc));
		}
	}
}


void GuiBibtex::rescanBibStyles() const
{
	if (usingBiblatex())
		rescanTexStyles("bib");
	else
		rescanTexStyles("bst bib");
}


void GuiBibtex::findText(QString const & text)
{
	QStringList const result = bibFiles(false).filter(text);
	available_model_.setStringList(result);
}


void GuiBibtex::filterChanged(const QString & text)
{
	if (!text.isEmpty()) {
		findText(filter_->text());
		return;
	}
	findText(filter_->text());
	filter_->setFocus();
}


void GuiBibtex::filterPressed()
{
	findText(filter_->text());
}


void GuiBibtex::resetFilter()
{
	filter_->setText(QString());
	findText(filter_->text());
}



bool GuiBibtex::usingBibtopic() const
{
	return buffer().params().useBibtopic();
}


bool GuiBibtex::bibtotoc() const
{
	return prefixIs(to_utf8(params_["options"]), "bibtotoc");
}


bool GuiBibtex::usingBiblatex() const
{
	return buffer().masterBuffer()->params().useBiblatex();
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


bool GuiBibtex::initialiseParams(std::string const & sdata)
{
	InsetCommand::string2params(sdata, params_);
	init();
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
