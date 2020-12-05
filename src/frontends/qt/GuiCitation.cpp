
/**
 * \file GuiCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCitation.h"

#include "FancyLineEdit.h"
#include "GuiApplication.h"
#include "GuiSelectionManager.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "Citation.h"
#include "TextClass.h"
#include "FuncRequest.h"

#include "insets/InsetCitation.h"
#include "insets/InsetCommand.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QCloseEvent>
#include <QMenu>
#include <QSettings>
#include <QShowEvent>
#include <QStandardItemModel>
#include <QVariant>

#undef KeyPress

#include <algorithm>
#include <regex>
#include <string>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

// FIXME THREAD
// I am guessing that it would not hurt to make these private members.
static vector<string> citeCmds_;
static vector<CitationStyle> citeStyles_;


template<typename String>
static QStringList to_qstring_list(vector<String> const & v)
{
	QStringList qlist;

	for (size_t i = 0; i != v.size(); ++i) {
		if (v[i].empty())
			continue;
		qlist.append(lyx::toqstr(v[i]));
	}
	return qlist;
}


static vector<lyx::docstring> to_docstring_vector(QStringList const & qlist)
{
	vector<lyx::docstring> v;
	for (int i = 0; i != qlist.size(); ++i) {
		if (qlist[i].isEmpty())
			continue;
		v.push_back(lyx::qstring_to_ucs4(qlist[i]));
	}
	return v;
}


GuiCitation::GuiCitation(GuiView & lv)
	: DialogView(lv, "citation", qt_("Citation")),
	  style_(QString()), params_(insetCode("citation"))
{
	setupUi(this);

	// The filter bar
	filter_ = new FancyLineEdit(this);
	filter_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	filter_->setButtonVisible(FancyLineEdit::Right, true);
	filter_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	filter_->setAutoHideButton(FancyLineEdit::Right, true);
	filter_->setPlaceholderText(qt_("All avail. citations"));

	filterBarL->addWidget(filter_, 0);
	findKeysLA->setBuddy(filter_);

	// Add search options as button menu
	regexp_ = new QAction(qt_("Regular e&xpression"), this);
	regexp_->setCheckable(true);
	casesense_ = new QAction(qt_("Case se&nsitive"), this);
	casesense_->setCheckable(true);
	instant_ = new QAction(qt_("Search as you &type"), this);
	instant_->setCheckable(true);
	instant_->setChecked(true);

	QMenu * searchOpts = new QMenu(this);
	searchOpts->addAction(regexp_);
	searchOpts->addAction(casesense_);
	searchOpts->addAction(instant_);
	searchOptionsPB->setMenu(searchOpts);

	connect(citationStyleCO, SIGNAL(activated(int)),
		this, SLOT(on_citationStyleCO_currentIndexChanged(int)));
	connect(starredCB, SIGNAL(clicked()),
		this, SLOT(updateStyles()));
	connect(literalCB, SIGNAL(clicked()),
		this, SLOT(changed()));
	connect(forceuppercaseCB, SIGNAL(clicked()),
		this, SLOT(updateStyles()));
	connect(textBeforeED, SIGNAL(textChanged(QString)),
		this, SLOT(updateStyles()));
	connect(textAfterED, SIGNAL(textChanged(QString)),
		this, SLOT(updateStyles()));
	connect(textBeforeED, SIGNAL(returnPressed()),
		this, SLOT(on_buttonBox_accepted()));
	connect(textAfterED, SIGNAL(returnPressed()),
		this, SLOT(on_buttonBox_accepted()));

	selectionManager = new GuiSelectionManager(this, availableLV, selectedLV,
			addPB, deletePB, upPB, downPB, &available_model_, &selected_model_, 1);
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(setCitedKeys()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateControls()));
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
	connect(regexp_, SIGNAL(triggered()),
		this, SLOT(regexChanged()));
	connect(casesense_, SIGNAL(triggered()),
		this, SLOT(caseChanged()));
	connect(instant_, SIGNAL(triggered(bool)),
		this, SLOT(instantChanged(bool)));

#if (QT_VERSION < 0x050000)
	selectedLV->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
	selectedLV->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif

	selectedLV->setToolTip(qt_("Ordered list of all cited references.\n"
				   "You can reorder, add and remove references with the buttons on the left."));
	setFocusProxy(filter_);
}


void GuiCitation::closeEvent(QCloseEvent * e)
{
	clearSelection();
	DialogView::closeEvent(e);
}


void GuiCitation::applyView()
{
	int const choice = max(0, citationStyleCO->currentIndex());
	style_ = citationStyleCO->itemData(citationStyleCO->currentIndex()).toString();
	bool const full  = starredCB->isChecked();
	bool const force = forceuppercaseCB->isChecked();

	QString const before = textBeforeED->text();
	QString const after = textAfterED->text();

	applyParams(choice, full, force, before, after);
}


void GuiCitation::showEvent(QShowEvent * e)
{
	if (!filter_->text().isEmpty())
		filterPressed();
	availableLV->setFocus();
	DialogView::showEvent(e);
}


void GuiCitation::on_buttonBox_accepted()
{
	applyView();
	clearSelection();
	hide();
}


void GuiCitation::on_buttonBox_rejected()
{
	clearSelection();
	hide();
}


void GuiCitation::on_buttonBox_clicked(QAbstractButton * button)
{
	switch (buttonBox->standardButton(button)) {
	case QDialogButtonBox::Apply:
		applyView();
		break;
	case QDialogButtonBox::Reset:
		init();
		updateFilterHint();
		filterPressed();
		break;
	default:
		break;
	}
}


void GuiCitation::on_literalCB_clicked()
{
	InsetCitation::last_literal = literalCB->isChecked();
	changed();
}


void GuiCitation::updateControls()
{
	BiblioInfo const & bi = bibInfo();
	updateControls(bi);
}


// The main point of separating this out is that the fill*() methods
// called in update() do not need to be called for INTERNAL updates,
// such as when addPB is pressed, as the list of fields, entries, etc,
// will not have changed.
void GuiCitation::updateControls(BiblioInfo const & bi)
{
	QModelIndex idx = selectionManager->getSelectedIndex(1);
	updateInfo(bi, idx);
	int i = citationStyleCO->currentIndex();
	if (i == -1 || i > int(citeStyles_.size()))
		i = 0;
	updateFormatting(citeStyles_[size_type(i)]);
	selectionManager->update();
}


void GuiCitation::updateFormatting(CitationStyle const & currentStyle)
{
	BufferParams const bp = documentBuffer().params();
	bool const force = currentStyle.forceUpperCase;
	bool const starred = currentStyle.hasStarredVersion;
	bool const full = starred && bp.fullAuthorList();
	bool const textbefore = currentStyle.textBefore;
	bool const textafter = currentStyle.textAfter;

	int const rows = selectedLV->model()->rowCount();

	bool const qualified = currentStyle.hasQualifiedList && rows > 0;

	selectedLV->horizontalHeader()->setVisible(qualified);
	selectedLV->setColumnHidden(0, !qualified);
	selectedLV->setColumnHidden(2, !qualified);
	bool const haveSelection = rows > 0;
	if (qualified) {
		textBeforeLA->setText(qt_("General text befo&re:"));
		textAfterLA->setText(qt_("General &text after:"));
		textBeforeED->setToolTip(qt_("Text that precedes the whole reference list. "
					     "For text that precedes individual items, "
					     "double-click on the respective entry above."));
		textAfterLA->setToolTip(qt_("General &text after:"));
		textAfterED->setToolTip(qt_("Text that follows the whole reference list. "
					     "For text that follows individual items, "
					     "double-click on the respective entry above."));
	} else {
		textBeforeLA->setText(qt_("Text befo&re:"));
		if (textbefore && haveSelection)
			textBeforeED->setToolTip(qt_("Text that precedes the reference (e.g., \"cf.\")"));
		else
			textBeforeED->setToolTip(qt_("Text that precedes the reference (e.g., \"cf.\"), "
						     "if the current citation style supports this."));
		textAfterLA->setText(qt_("&Text after:"));
		if (textafter && haveSelection)
			textAfterED->setToolTip(qt_("Text that follows the reference (e.g., pages)"));
		else
			textAfterED->setToolTip(qt_("Text that follows the reference (e.g., pages), "
						    "if the current citation style supports this."));
	}

	forceuppercaseCB->setEnabled(force && haveSelection);
	if (force && haveSelection)
		forceuppercaseCB->setToolTip(qt_("Force upper case in names (\"Del Piero\", not \"del Piero\")."));
	else
		forceuppercaseCB->setToolTip(qt_("Force upper case in names (\"Del Piero\", not \"del Piero\"), "
					     "if the current citation style supports this."));
	starredCB->setEnabled(full && haveSelection);
	textBeforeED->setEnabled(textbefore && haveSelection);
	textBeforeLA->setEnabled(textbefore && haveSelection);
	textAfterED->setEnabled(textafter && haveSelection);
	textAfterLA->setEnabled(textafter && haveSelection);
	literalCB->setEnabled(textbefore || textafter);
	citationStyleCO->setEnabled(haveSelection);
	citationStyleLA->setEnabled(haveSelection);

	// Check if we have a custom string/tooltip for the starred version
	if (starred && !currentStyle.stardesc.empty()) {
		string val =
			bp.documentClass().getCiteMacro(bp.citeEngineType(), currentStyle.stardesc);
		docstring guistring;
		if (!val.empty()) {
			guistring = translateIfPossible(from_utf8(val));
			starredCB->setText(toqstr(guistring));
			starredCB->setEnabled(haveSelection);
		}
		if (!currentStyle.startooltip.empty()) {
			val = bp.documentClass().getCiteMacro(bp.citeEngineType(),
							      currentStyle.startooltip);
			if (!val.empty())
				guistring = translateIfPossible(from_utf8(val));
		}
		// Tooltip might also be empty
		starredCB->setToolTip(toqstr(guistring));
	} else {
		// This is the default meaning of the starred commands
		starredCB->setText(qt_("All aut&hors"));
		if (full && haveSelection)
			starredCB->setToolTip(qt_("Always list all authors (rather than using \"et al.\")"));
		else
			starredCB->setToolTip(qt_("Always list all authors (rather than using \"et al.\"), "
						  "if the current citation style supports this."));
	}
	if (availableLV->selectionModel()->selectedIndexes().isEmpty())
		availableLV->setToolTip(qt_("All references available for citing."));
	else
		availableLV->setToolTip(qt_("All references available for citing.\n"
					    "To add the selected one, hit Add, press Enter or double-click.\n"
					    "Hit Ctrl-Enter to add and close the dialog."));
	// With qualified citation lists, it makes sense to add the same key multiple times
	selectionManager->allowMultiSelection(currentStyle.hasQualifiedList);
}


// Update the styles for the style combo, citationStyleCO, and mark the
// settings as changed. Called upon changing the cited keys (including
// merely reordering the keys) or editing the text before/after fields.
void GuiCitation::updateStyles()
{
	BiblioInfo const & bi = bibInfo();
	updateStyles(bi);
	changed();
}


// Update the styles for the style combo, citationStyleCO.
void GuiCitation::updateStyles(BiblioInfo const & bi)
{
	QStringList selected_keys = selectedKeys();
	int curr = selectedLV->model()->rowCount() - 1;

	if (curr < 0 || selected_keys.empty()) {
		last_chosen_style_ =
			citationStyleCO->itemData(citationStyleCO->currentIndex()).toString();
		citationStyleCO->clear();
		citationStyleCO->setEnabled(false);
		citationStyleLA->setEnabled(false);
		return;
	}

	static const size_t max_length = 80;
	BiblioInfo::CiteStringMap sty = citationStyles(bi, max_length);

	if (sty.empty()) {
		// some error
		citationStyleCO->setEnabled(false);
		citationStyleLA->setEnabled(false);
		citationStyleCO->clear();
		return;
	}

	citationStyleCO->blockSignals(true);

	// save old style selection
	QString const curdata =
		citationStyleCO->itemData(citationStyleCO->currentIndex()).toString();
	QString const olddata = (curdata.isEmpty()) ?
		(last_chosen_style_.isEmpty() ? style_ : last_chosen_style_): curdata;
	// reset this
	last_chosen_style_.clear();
	citationStyleCO->clear();
	BiblioInfo::CiteStringMap::const_iterator cit = sty.begin();
	BiblioInfo::CiteStringMap::const_iterator end = sty.end();
	for (int ii = 1; cit != end; ++cit, ++ii)
		citationStyleCO->addItem(toqstr(cit->second), toqstr(cit->first));
	citationStyleCO->setEnabled(true);
	citationStyleLA->setEnabled(true);
	// restore old style selection
	int const i = citationStyleCO->findData(olddata);
	if (i != -1)
		citationStyleCO->setCurrentIndex(i);

	citationStyleCO->blockSignals(false);
}


void GuiCitation::fillFields(BiblioInfo const & bi)
{
	fieldsCO->blockSignals(true);
	int const oldIndex = fieldsCO->currentIndex();
	fieldsCO->clear();
	QStringList const fields = to_qstring_list(bi.getFields());
	fieldsCO->insertItem(0, qt_("All fields"));
	fieldsCO->insertItem(1, qt_("Keys"));
	fieldsCO->insertItems(2, fields);
	if (oldIndex != -1 && oldIndex < fieldsCO->count())
		fieldsCO->setCurrentIndex(oldIndex);
	fieldsCO->blockSignals(false);
}


void GuiCitation::fillEntries(BiblioInfo const & bi)
{
	entriesCO->blockSignals(true);
	int const oldIndex = entriesCO->currentIndex();
	entriesCO->clear();
	QStringList const entries = to_qstring_list(bi.getEntries());
	entriesCO->insertItem(0, qt_("All entry types"));
	entriesCO->insertItems(1, entries);
	if (oldIndex != -1 && oldIndex < entriesCO->count())
		entriesCO->setCurrentIndex(oldIndex);
	entriesCO->blockSignals(false);
}


bool GuiCitation::isSelected(QModelIndex const & idx)
{
	QString const str = idx.data().toString();
	return selectedKeys().contains(str);
}


void GuiCitation::setButtons()
{
	int const srows = selectedLV->model()->rowCount();
	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(srows > 0);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(srows > 0);
}


void GuiCitation::updateInfo(BiblioInfo const & bi, QModelIndex const & idx)
{
	if (!idx.isValid() || bi.empty()) {
		infoML->document()->clear();
		infoML->setToolTip(qt_("Displays a sketchy preview if a citation is selected above"));
		return;
	}

	infoML->setToolTip(qt_("Sketchy preview of the selected citation"));
	CiteItem ci;
	ci.richtext = true;
	QString const keytxt = toqstr(
		bi.getInfo(qstring_to_ucs4(idx.data().toString()), documentBuffer(), ci));
	infoML->document()->setHtml(keytxt);
}


void GuiCitation::findText(QString const & text, bool reset)
{
	//"All Fields" and "Keys" are the first two
	int index = fieldsCO->currentIndex() - 2;
	BiblioInfo const & bi = bibInfo();
	vector<docstring> const & fields = bi.getFields();
	docstring field;

	if (index <= -1 || index >= int(fields.size()))
		//either "All Fields" or "Keys" or an invalid value
		field = from_ascii("");
	else
		field = fields[size_type(index)];

	//Was it "Keys"?
	bool const onlyKeys = index == -1;

	//"All Entry Types" is first.
	index = entriesCO->currentIndex() - 1;
	vector<docstring> const & entries = bi.getEntries();
	docstring entry_type;
	if (index < 0 || index >= int(entries.size()))
		entry_type = from_ascii("");
	else
		entry_type = entries[size_type(index)];

	bool const case_sentitive = casesense_->isChecked();
	bool const reg_exp = regexp_->isChecked();

	findKey(bi, text, onlyKeys, field, entry_type,
	               case_sentitive, reg_exp, reset);
	//FIXME
	//It'd be nice to save and restore the current selection in
	//availableLV. Currently, we get an automatic reset, since the
	//model is reset.

	updateControls(bi);
}


void GuiCitation::on_fieldsCO_currentIndexChanged(int /*index*/)
{
	findText(filter_->text(), true);
}


void GuiCitation::on_entriesCO_currentIndexChanged(int /*index*/)
{
	findText(filter_->text(), true);
}


void GuiCitation::on_citationStyleCO_currentIndexChanged(int index)
{
	if (index >= 0 && index < citationStyleCO->count()) {
		vector<CitationStyle> const & styles = citeStyles_;
		updateFormatting(styles[size_type(index)]);
		changed();
	}
}


void GuiCitation::filterChanged(const QString & text)
{
	if (!text.isEmpty()) {
		if (instant_->isChecked())
			findText(filter_->text());
		return;
	}
	findText(filter_->text());
	filter_->setFocus();
}


void GuiCitation::filterPressed()
{
	findText(filter_->text(), true);
}


void GuiCitation::resetFilter()
{
	filter_->setText(QString());
	findText(filter_->text(), true);
}


void GuiCitation::caseChanged()
{
	findText(filter_->text());
}


void GuiCitation::regexChanged()
{
	findText(filter_->text());
}


void GuiCitation::updateFilterHint()
{
	QString hint = instant_->isChecked() ?
		qt_("Enter string to filter the list of available citations") :
		qt_("Enter string to filter the list of available citations and press <Enter>");
	hint += qt_("\nThe down arrow key will get you into the list of filtered citations.");
	filter_->setToolTip(hint);
}


void GuiCitation::instantChanged(bool checked)
{
	if (checked)
		findText(filter_->text(), true);

	updateFilterHint();
}


void GuiCitation::changed()
{
	setButtons();
}


void GuiCitation::applyParams(int const choice, bool full, bool force,
	QString before, QString after)
{
	if (cited_keys_.isEmpty())
		return;

	vector<CitationStyle> const & styles = citeStyles_;

	CitationStyle cs = styles[size_type(choice)];

	if (!cs.textBefore)
		before.clear();
	if (!cs.textAfter)
		after.clear();

	cs.forceUpperCase &= force;
	cs.hasStarredVersion &= full;
	string const command = citationStyleToString(cs);

	params_.setCmdName(command);
	params_["key"] = qstring_to_ucs4(cited_keys_.join(","));
	params_["before"] = qstring_to_ucs4(before);
	params_["after"] = qstring_to_ucs4(after);
	if (cs.hasQualifiedList) {
		params_["pretextlist"] = getStringFromVector(getPreTexts(), from_ascii("\t"));
		params_["posttextlist"] = getStringFromVector(getPostTexts(), from_ascii("\t"));
	}
	params_["literal"] = literalCB->isChecked() ? from_ascii("true") : from_ascii("false");
	dispatchParams();
}


void GuiCitation::clearSelection()
{
	cited_keys_.clear();
	setSelectedKeys(cited_keys_);
}


void GuiCitation::setSelectedKeys(QStringList const & sl)
{
	selected_model_.clear();
	selected_model_.setColumnCount(3);
	QStringList headers;
	headers << qt_("Text before")
		<< qt_("Cite key")
		<< qt_("Text after");
	selected_model_.setHorizontalHeaderLabels(headers);
	selectedLV->setColumnHidden(0, true);
	selectedLV->setColumnHidden(2, true);
	selectedLV->verticalHeader()->setVisible(false);
	selectedLV->horizontalHeader()->setVisible(false);
	QStringList::const_iterator it  = sl.begin();
	QStringList::const_iterator end = sl.end();
	for (int i = 0; it != end; ++it, ++i) {
		QStandardItem * si = new QStandardItem();
		si->setData(*it);
		si->setText(*it);
		si->setToolTip(*it);
		si->setEditable(false);
		selected_model_.setItem(i, 1, si);
	}
}


QStringList GuiCitation::selectedKeys()
{
	QStringList res;
	for (int i = 0; i != selected_model_.rowCount(); ++i) {
		QStandardItem const * item = selected_model_.item(i, 1);
		if (item)
			res.append(item->text());
	}
	return res;
}


void GuiCitation::setPreTexts(vector<docstring> const & m)
{
	// account for multiple use of the same keys
	QList<QModelIndex> handled;
	for (docstring const & s: m) {
		QStandardItem * si = new QStandardItem();
		docstring key;
		docstring pre = split(s, key, ' ');
		si->setData(toqstr(pre));
		si->setText(toqstr(pre));
		QModelIndexList qmil =
			selected_model_.match(selected_model_.index(0, 1),
					     Qt::DisplayRole, toqstr(key), -1,
					     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
		for (auto const & idx : qmil) {
			if (!handled.contains(idx)) {
				selected_model_.setItem(idx.row(), 0, si);
				handled.append(idx);
				break;
			}
		}
	}
}


vector<docstring> GuiCitation::getPreTexts()
{
	vector<docstring> res;
	for (int i = 0; i != selected_model_.rowCount(); ++i) {
		QStandardItem const * key = selected_model_.item(i, 1);
		QStandardItem const * pre = selected_model_.item(i, 0);
		if (key && pre && !key->text().isEmpty())
			res.push_back(qstring_to_ucs4(key->text()) + " " + qstring_to_ucs4(pre->text()));
	}
	return res;
}


void GuiCitation::setPostTexts(vector<docstring> const & m)
{
	// account for multiple use of the same keys
	QList<QModelIndex> handled;
	for (docstring const & s: m) {
		QStandardItem * si = new QStandardItem();
		docstring key;
		docstring post = split(s, key, ' ');
		si->setData(toqstr(post));
		si->setText(toqstr(post));
		QModelIndexList qmil =
			selected_model_.match(selected_model_.index(0, 1),
					     Qt::DisplayRole, toqstr(key), -1,
					     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
		for (auto const & idx : qmil) {
			if (!handled.contains(idx)) {
				selected_model_.setItem(idx.row(), 2, si);
				handled.append(idx);
				break;
			}
		}
	}
}


vector<docstring> GuiCitation::getPostTexts()
{
	vector<docstring> res;
	for (int i = 0; i != selected_model_.rowCount(); ++i) {
		QStandardItem const * key = selected_model_.item(i, 1);
		QStandardItem const * post = selected_model_.item(i, 2);
		if (key && post && !key->text().isEmpty())
			res.push_back(qstring_to_ucs4(key->text()) + " " + qstring_to_ucs4(post->text()));
	}
	return res;
}


void GuiCitation::init()
{
	// Make the list of all available bibliography keys
	BiblioInfo const & bi = bibInfo();
	all_keys_ = to_qstring_list(bi.getKeys());

	available_model_.setStringList(all_keys_);

	// Ditto for the keys cited in this inset
	QString str = toqstr(params_["key"]);
	if (str.isEmpty())
		cited_keys_.clear();
	else
		cited_keys_ = str.split(",");
	setSelectedKeys(cited_keys_);

	// Initialize the drop downs
	fillEntries(bi);
	fillFields(bi);

	// Initialize the citation formatting
	string const & cmd = params_.getCmdName();
	CitationStyle const cs =
		citationStyleFromString(cmd, documentBuffer().params());

	forceuppercaseCB->setChecked(cs.forceUpperCase);
	starredCB->setChecked(cs.hasStarredVersion &&
		documentBuffer().params().fullAuthorList());
	textBeforeED->setText(toqstr(params_["before"]));
	textAfterED->setText(toqstr(params_["after"]));

	// if this is a new citation, we set the literal checkbox
	// to its last set value.
	if (cited_keys_.isEmpty())
		literalCB->setChecked(InsetCitation::last_literal);
	else
		literalCB->setChecked(params_["literal"] == "true");

	setPreTexts(getVectorFromString(params_["pretextlist"], from_ascii("\t")));
	setPostTexts(getVectorFromString(params_["posttextlist"], from_ascii("\t")));

	// Update the interface
	updateControls(bi);
	updateStyles(bi);
	if (selected_model_.rowCount()) {
		selectedLV->blockSignals(true);
		selectedLV->setFocus();
		selectedLV->selectRow(0);
		selectedLV->blockSignals(false);

		// Find the citation style
		vector<string> const & cmds = citeCmds_;
		vector<string>::const_iterator cit =
			std::find(cmds.begin(), cmds.end(), cs.name);
		int i = 0;
		if (cit != cmds.end())
			i = int(cit - cmds.begin());

		// Set the style combo appropriately
		citationStyleCO->blockSignals(true);
		citationStyleCO->setCurrentIndex(i);
		citationStyleCO->blockSignals(false);
		updateFormatting(citeStyles_[size_type(i)]);
	} else
		availableLV->setFocus();

	buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


void GuiCitation::findKey(BiblioInfo const & bi,
	QString const & str, bool only_keys,
	docstring field, docstring entry_type,
	bool case_sensitive, bool reg_exp, bool reset)
{
	// FIXME THREAD
	// This should be moved to a class member.
	// Used for optimisation: store last searched string.
	static QString last_searched_string;
	// Used to disable the above optimisation.
	static bool last_case_sensitive;
	static bool last_reg_exp;
	// Reset last_searched_string in case of changed option.
	if (last_case_sensitive != case_sensitive
		|| last_reg_exp != reg_exp) {
			LYXERR(Debug::GUI, "GuiCitation::findKey: optimisation disabled!");
		last_searched_string.clear();
	}
	// save option for next search.
	last_case_sensitive = case_sensitive;
	last_reg_exp = reg_exp;

	Qt::CaseSensitivity qtcase = case_sensitive ?
			Qt::CaseSensitive: Qt::CaseInsensitive;
	QStringList keys;
	// If new string (str) contains the last searched one...
	if (!reset &&
		!last_searched_string.isEmpty() &&
		str.size() > 1 &&
		str.contains(last_searched_string, qtcase))
		// ... then only search within already found list.
		keys = available_model_.stringList();
	else
		// ... else search all keys.
		keys = all_keys_;
	// save searched string for next search.
	last_searched_string = str;

	QStringList result;

	// First, filter by entry_type, which will be faster than
	// what follows, so we may get to do that on less.
	vector<docstring> keyVector = to_docstring_vector(keys);
	filterByEntryType(bi, keyVector, entry_type);

	if (str.isEmpty())
		result = to_qstring_list(keyVector);
	else
		result = to_qstring_list(searchKeys(bi, keyVector, only_keys,
			qstring_to_ucs4(str), field, case_sensitive, reg_exp));

	available_model_.setStringList(result);
}


BiblioInfo::CiteStringMap GuiCitation::citationStyles(BiblioInfo const & bi, size_t max_size)
{
	vector<docstring> const keys = to_docstring_vector(cited_keys_);
	vector<CitationStyle> styles = citeStyles_;
	int ind = citationStyleCO->currentIndex();
	if (ind == -1)
		ind = 0;
	CitationStyle cs = styles[size_type(ind)];
	vector<docstring> pretexts = getPreTexts();
	vector<docstring> posttexts = getPostTexts();
	bool const qualified = cs.hasQualifiedList
		&& (selectedLV->model()->rowCount() > 1
		    || !pretexts.empty()
		    || !posttexts.empty());
	vector<pair<docstring, docstring>> pres;
	for (docstring const & s: pretexts) {
		docstring key;
		docstring val = split(s, key, ' ');
		pres.push_back(make_pair(key, val));
	}
	vector<pair<docstring, docstring>> posts;
	for (docstring const & s: posttexts) {
		docstring key;
		docstring val = split(s, key, ' ');
		posts.push_back(make_pair(key, val));
	}
	CiteItem ci;
	ci.textBefore = qstring_to_ucs4(textBeforeED->text());
	ci.textAfter = qstring_to_ucs4(textAfterED->text());
	ci.forceUpperCase = forceuppercaseCB->isChecked();
	ci.Starred = starredCB->isChecked();
	ci.context = CiteItem::Dialog;
	ci.max_size = max_size;
	ci.isQualified = qualified;
	ci.pretexts = pres;
	ci.posttexts = posts;
	BiblioInfo::CiteStringMap ret = bi.getCiteStrings(keys, styles, documentBuffer(), ci);
	return ret;
}


void GuiCitation::setCitedKeys()
{
	cited_keys_ = selectedKeys();
	updateStyles();
}


bool GuiCitation::initialiseParams(string const & sdata)
{
	InsetCommand::string2params(sdata, params_);
	citeCmds_ = documentBuffer().params().citeCommands();
	citeStyles_ = documentBuffer().params().citeStyles();
	init();
	return true;
}


void GuiCitation::clearParams()
{
	params_.clear();
}


void GuiCitation::filterByEntryType(BiblioInfo const & bi,
	vector<docstring> & keyVector, docstring const & entry_type)
{
	if (entry_type.empty())
		return;

	vector<docstring>::iterator it = keyVector.begin();
	vector<docstring>::iterator end = keyVector.end();

	vector<docstring> result;
	for (; it != end; ++it) {
		docstring const key = *it;
		BiblioInfo::const_iterator cit = bi.find(key);
		if (cit == bi.end())
			continue;
		if (cit->second.entryType() == entry_type)
			result.push_back(key);
	}
	keyVector = result;
}


// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
// @todo: This function should be moved to support, and then the test in tests
//        should be moved there as well.
static docstring escape_special_chars(docstring const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[', ']', and '\' must be escaped.
	static const regex reg("[.|*?+(){}^$\\[\\]\\\\]");

	// $& is an ECMAScript format expression that expands to all
	// of the current match
	// To prefix a matched expression with a single literal backslash, we
	// need to escape it for the C++ compiler and use:
	// FIXME: UNICODE
	return from_utf8(regex_replace(to_utf8(expr), reg, string("\\$&")));
}


vector<docstring> GuiCitation::searchKeys(BiblioInfo const & bi,
	vector<docstring> const & keys_to_search, bool only_keys,
 	docstring const & search_expression, docstring const & field,
	bool case_sensitive, bool re)
{
	vector<docstring> foundKeys;

	docstring expr = trim(search_expression);
	if (expr.empty())
		return foundKeys;

	if (!re)
		// We must escape special chars in the search_expr so that
		// it is treated as a simple string by regex.
		expr = escape_special_chars(expr);

	regex reg_exp;
	try {
		reg_exp.assign(to_utf8(expr), case_sensitive ?
			regex_constants::ECMAScript : regex_constants::icase);
	} catch (regex_error const & e) {
		// regex throws an exception if the regular expression is not
		// valid.
		LYXERR(Debug::GUI, e.what());
		return vector<docstring>();
	}

	vector<docstring>::const_iterator it = keys_to_search.begin();
	vector<docstring>::const_iterator end = keys_to_search.end();
	for (; it != end; ++it ) {
		BiblioInfo::const_iterator info = bi.find(*it);
		if (info == bi.end())
			continue;

		BibTeXInfo const & kvm = info->second;
		string sdata;
		if (only_keys)
			sdata = to_utf8(*it);
		else if (field.empty())
			sdata = to_utf8(*it) + ' ' + to_utf8(kvm.allData());
		else
			sdata = to_utf8(kvm[field]);

		if (sdata.empty())
			continue;

		try {
			if (regex_search(sdata, reg_exp))
				foundKeys.push_back(*it);
		}
		catch (regex_error const & e) {
			LYXERR(Debug::GUI, e.what());
			return vector<docstring>();
		}
	}
	return foundKeys;
}


void GuiCitation::dispatchParams()
{
	std::string const lfun = InsetCommand::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


BiblioInfo const & GuiCitation::bibInfo() const
{
	Buffer const & buf = documentBuffer();
	buf.reloadBibInfoCache();
	return buf.masterBibInfo();
}


void GuiCitation::saveSession(QSettings & settings) const
{
	Dialog::saveSession(settings);
	settings.setValue(
		sessionKey() + "/regex", regexp_->isChecked());
	settings.setValue(
		sessionKey() + "/casesensitive", casesense_->isChecked());
	settings.setValue(
		sessionKey() + "/autofind", instant_->isChecked());
	settings.setValue(
		sessionKey() + "/citestyle", style_);
	settings.setValue(
		sessionKey() + "/literal", InsetCitation::last_literal);
}


void GuiCitation::restoreSession()
{
	Dialog::restoreSession();
	QSettings settings;
	regexp_->setChecked(settings.value(sessionKey() + "/regex").toBool());
	casesense_->setChecked(settings.value(sessionKey() + "/casesensitive").toBool());
	instant_->setChecked(settings.value(sessionKey() + "/autofind", true).toBool());
	style_ = settings.value(sessionKey() + "/citestyle").toString();
	InsetCitation::last_literal = 
		settings.value(sessionKey() + "/literal", false).toBool();
	updateFilterHint();
}


Dialog * createGuiCitation(GuiView & lv) { return new GuiCitation(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiCitation.cpp"

