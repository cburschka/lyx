/**
 * \file GuiCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCitation.h"

#include "debug.h"
#include "gettext.h"
#include "frontend_helpers.h"
#include "qt_helpers.h"
#include "Buffer.h"
#include "BufferParams.h"

#include "support/lstrings.h"
#include "support/docstring.h"

#include <vector>
#include <string>

#include <QCloseEvent>

#undef KeyPress

#include <boost/regex.hpp>

#include <algorithm>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::pair;

namespace lyx {
namespace frontend {

static vector<biblio::CiteStyle> citeStyles_;


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


GuiCitation::GuiCitation(LyXView & lv)
	: GuiDialog(lv, "citation"), ControlCommand(*this, "citation")
{
	setupUi(this);
	setViewTitle(_("Citation"));
	setController(this, false);

	connect(citationStyleCO, SIGNAL(activated(int)),
		this, SLOT(changed()));
	connect(fulllistCB, SIGNAL(clicked()),
		this, SLOT(changed()));
	connect(forceuppercaseCB, SIGNAL(clicked()),
		this, SLOT(changed()));
	connect(textBeforeED, SIGNAL(textChanged(QString)),
		this, SLOT(changed()));
	connect(textAfterED, SIGNAL(textChanged(QString)),
		this, SLOT(changed()));
	connect(clearPB, SIGNAL(clicked()),
		findLE, SLOT(clear()));
	connect(this, SIGNAL(rejected()), this, SLOT(cleanUp()));

	selectionManager = new GuiSelectionManager(availableLV, selectedLV, 
			addPB, deletePB, upPB, downPB, available(), selected());
	connect(selectionManager, SIGNAL(selectionChanged()),
		this, SLOT(setCitedKeys()));
	connect(selectionManager, SIGNAL(updateHook()),
		this, SLOT(updateDialog()));
	connect(selectionManager, SIGNAL(okHook()),
		this, SLOT(on_okPB_clicked()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
}


void GuiCitation::cleanUp() 
{
	clearSelection();
	clearParams();
	close();
}


void GuiCitation::closeEvent(QCloseEvent * e)
{
	clearSelection();
	clearParams();
	GuiDialog::closeEvent(e);
}


void GuiCitation::applyView()
{
	int  const choice = std::max(0, citationStyleCO->currentIndex());
	style_ = choice;
	bool const full  = fulllistCB->isChecked();
	bool const force = forceuppercaseCB->isChecked();

	QString const before = textBeforeED->text();
	QString const after = textAfterED->text();

	apply(choice, full, force, before, after);
}


void GuiCitation::hideView()
{
	clearParams();
	accept();
}


void GuiCitation::showView()
{
	init();
	findLE->clear();
	availableLV->setFocus();
	QDialog::show();
	raise();
	activateWindow();
}


bool GuiCitation::isVisibleView() const
{
	return QDialog::isVisible();
}


void GuiCitation::on_okPB_clicked()
{
	applyView();
	clearSelection();
	hideView();
}


void GuiCitation::on_cancelPB_clicked()
{
	clearSelection();
	hideView();
}


void GuiCitation::on_applyPB_clicked()
{
	applyView();
}


void GuiCitation::on_restorePB_clicked()
{
	init();
	updateView();
}


void GuiCitation::updateView()
{
	init();
	fillFields();
	fillEntries();
	updateDialog();
}


// The main point of separating this out is that the fill*() methods
// called in update() do not need to be called for INTERNAL updates,
// such as when addPB is pressed, as the list of fields, entries, etc,
// will not have changed. At the moment, however, the division between
// fillStyles() and updateStyles() doesn't lend itself to dividing the
// two methods, though they should be divisible.
void GuiCitation::updateDialog()
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

	textBeforeED->setText(textBefore());
	textAfterED->setText(textAfter());
	fillStyles();
	updateStyle();
}


void GuiCitation::updateStyle()
{
	biblio::CiteEngine const engine = getEngine();
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

	string const & command = params().getCmdName();

	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles = citeStyles_;
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
void GuiCitation::fillStyles()
{
	int const oldIndex = citationStyleCO->currentIndex();

	citationStyleCO->clear();

	QStringList selected_keys = selected()->stringList();
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

	QStringList sty = citationStyles(curr);

	bool const basic_engine = (getEngine() == biblio::ENGINE_BASIC);

	citationStyleCO->setEnabled(!sty.isEmpty() && !basic_engine);
	citationStyleLA->setEnabled(!sty.isEmpty() && !basic_engine);

	if (sty.isEmpty() || basic_engine)
		return;

	citationStyleCO->insertItems(0, sty);

	if (oldIndex != -1 && oldIndex < citationStyleCO->count())
		citationStyleCO->setCurrentIndex(oldIndex);
}


void GuiCitation::fillFields()
{
	fieldsCO->blockSignals(true);
	int const oldIndex = fieldsCO->currentIndex();
	fieldsCO->clear();
	QStringList const & fields = getFieldsAsQStringList();
	fieldsCO->insertItem(0, qt_("All Fields"));
	fieldsCO->insertItem(1, qt_("Keys"));
	fieldsCO->insertItems(2, fields);
	if (oldIndex != -1 && oldIndex < fieldsCO->count())
		fieldsCO->setCurrentIndex(oldIndex);
	fieldsCO->blockSignals(false);
}


void GuiCitation::fillEntries()
{
	entriesCO->blockSignals(true);
	int const oldIndex = entriesCO->currentIndex();
	entriesCO->clear();
	QStringList const & entries = getEntriesAsQStringList();
	entriesCO->insertItem(0, qt_("All Entry Types"));
	entriesCO->insertItems(1, entries);
	if (oldIndex != -1 && oldIndex < entriesCO->count())
		entriesCO->setCurrentIndex(oldIndex);
	entriesCO->blockSignals(false);
}


bool GuiCitation::isSelected(const QModelIndex & idx)
{
	QString const str = idx.data().toString();
	return selected()->stringList().contains(str);
}


void GuiCitation::setButtons()
{
	selectionManager->update();
	int const srows = selectedLV->model()->rowCount();
	applyPB->setEnabled(srows > 0);
	okPB->setEnabled(srows > 0);
}


void GuiCitation::updateInfo(QModelIndex const & idx)
{
	if (idx.isValid()) {
		QString const keytxt = getKeyInfo(idx.data().toString());
		infoML->document()->setPlainText(keytxt);
	} else
		infoML->document()->clear();
}


void GuiCitation::findText(QString const & text, bool reset)
{
	//"All Fields" and "Keys" are the first two
	int index = fieldsCO->currentIndex() - 2; 
	vector<docstring> const & fields = availableFields();
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
	vector<docstring> const & entries = availableEntries();
	docstring entryType;
	if (index < 0 || index >= int(entries.size()))
		entryType = from_ascii("");
	else 
		entryType = entries[index];
	
	bool const case_sentitive = caseCB->checkState();
	bool const reg_exp = regexCB->checkState();
	findKey(text, onlyKeys, field, entryType, 
	               case_sentitive, reg_exp, reset);
	//FIXME
	//It'd be nice to save and restore the current selection in 
	//availableLV. Currently, we get an automatic reset, since the
	//model is reset.
	
	updateDialog();
}


void GuiCitation::on_fieldsCO_currentIndexChanged(int /*index*/)
{
	findText(findLE->text(), true);
}


void GuiCitation::on_entriesCO_currentIndexChanged(int /*index*/)
{
	findText(findLE->text(), true);
}


void GuiCitation::on_findLE_textChanged(const QString & text)
{
	clearPB->setDisabled(text.isEmpty());
	if (text.isEmpty())
		findLE->setFocus();
	findText(text);
}


void GuiCitation::on_caseCB_stateChanged(int)
{
	findText(findLE->text());
}


void GuiCitation::on_regexCB_stateChanged(int)
{
	findText(findLE->text());
}


void GuiCitation::changed()
{
	fillStyles();
	setButtons();
}


void GuiCitation::apply(int const choice, bool const full, bool const force,
	QString before, QString after)
{
	if (cited_keys_.isEmpty())
		return;

	vector<biblio::CiteStyle> const & styles = citeStyles_;

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	params().setCmdName(command);
	params()["key"] = qstring_to_ucs4(cited_keys_.join(","));
	params()["before"] = qstring_to_ucs4(before);
	params()["after"] = qstring_to_ucs4(after);
	dispatchParams();
}


void GuiCitation::clearSelection()
{
	cited_keys_.clear();
	selected_model_.setStringList(cited_keys_);
}


QString GuiCitation::textBefore()
{
	return toqstr(params()["before"]);
}


QString GuiCitation::textAfter()
{
	return toqstr(params()["after"]);
}


void GuiCitation::init()
{
	// Make the list of all available bibliography keys
	all_keys_ = to_qstring_list(availableKeys());
	available_model_.setStringList(all_keys_);

	// Ditto for the keys cited in this inset
	QString str = toqstr(params()["key"]);
	if (str.isEmpty())
		cited_keys_.clear();
	else
		cited_keys_ = str.split(",");
	selected_model_.setStringList(cited_keys_);
}


void GuiCitation::findKey(QString const & str, bool only_keys,
	docstring field, docstring entryType,
	bool case_sensitive, bool reg_exp, bool reset)
{
	// Used for optimisation: store last searched string.
	static QString last_searched_string;
	// Used to disable the above optimisation.
	static bool last_case_sensitive;
	static bool last_reg_exp;
	// Reset last_searched_string in case of changed option.
	if (last_case_sensitive != case_sensitive
		|| last_reg_exp != reg_exp) {
			LYXERR(Debug::GUI) << "GuiCitation::findKey: optimisation disabled!" << std::endl;
		last_searched_string.clear();
	}
	// save option for next search.
	last_case_sensitive = case_sensitive;
	last_reg_exp = reg_exp;

	Qt::CaseSensitivity qtcase = case_sensitive?
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
	
	// First, filter by entryType, which will be faster than 
	// what follows, so we may get to do that on less.
	vector<docstring> keyVector = to_docstring_vector(keys);
	filterByEntryType(keyVector, entryType);
	
	if (str.isEmpty())
		result = to_qstring_list(keyVector);
	else
		result = to_qstring_list(searchKeys(keyVector, only_keys, 
			qstring_to_ucs4(str), field, case_sensitive, reg_exp));
	
	available_model_.setStringList(result);
}


QStringList GuiCitation::getFieldsAsQStringList()
{
	return to_qstring_list(availableFields());
}


QStringList GuiCitation::getEntriesAsQStringList()
{
	return to_qstring_list(availableEntries());
}


QStringList GuiCitation::citationStyles(int sel)
{
	docstring const key = qstring_to_ucs4(cited_keys_[sel]);
	return to_qstring_list(bibkeysInfo_.getCiteStrings(key, buffer()));
}


QString GuiCitation::getKeyInfo(QString const & sel)
{
	return toqstr(getInfo(qstring_to_ucs4(sel)));
}


void GuiCitation::setCitedKeys() 
{
	cited_keys_ = selected_model_.stringList();
}


bool GuiCitation::initialiseParams(string const & data)
{
	if (!ControlCommand::initialiseParams(data))
		return false;

	biblio::CiteEngine const engine = buffer().params().getEngine();

	bool use_styles = engine != biblio::ENGINE_BASIC;

	bibkeysInfo_.fillWithBibKeys(&buffer());
	
	if (citeStyles_.empty())
		citeStyles_ = biblio::getCiteStyles(engine);
	else {
		if ((use_styles && citeStyles_.size() == 1) ||
		    (!use_styles && citeStyles_.size() != 1))
			citeStyles_ = biblio::getCiteStyles(engine);
	}

	return true;
}



void GuiCitation::clearParams()
{
	ControlCommand::clearParams();
	bibkeysInfo_.clear();
}


vector<docstring> const GuiCitation::availableKeys() const
{
	return bibkeysInfo_.getKeys();
}


vector<docstring> const GuiCitation::availableFields() const
{
	return bibkeysInfo_.getFields();
}


vector<docstring> const GuiCitation::availableEntries() const
{
	return bibkeysInfo_.getEntries();
}


void GuiCitation::filterByEntryType(
	vector<docstring> & keyVector, docstring entryType) 
{
	if (entryType.empty())
		return;
	
	vector<docstring>::iterator it = keyVector.begin();
	vector<docstring>::iterator end = keyVector.end();
	
	vector<docstring> result;
	for (; it != end; ++it) {
		docstring const key = *it;
		BiblioInfo::const_iterator cit = bibkeysInfo_.find(key);
		if (cit == bibkeysInfo_.end())
			continue;
		if (cit->second.entryType == entryType)
			result.push_back(key);
	}
	keyVector = result;
}


biblio::CiteEngine GuiCitation::getEngine() const
{
	return buffer().params().getEngine();
}


docstring GuiCitation::getInfo(docstring const & key) const
{
	if (bibkeysInfo_.empty())
		return docstring();

	return bibkeysInfo_.getInfo(key);
}


// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
// @todo: This function should be moved to support, and then the test in tests
//        should be moved there as well.
static docstring escape_special_chars(docstring const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[' and '\' must be escaped.
	// This is a limitation of boost::regex, but all other chars in BREs
	// are assumed literal.
	static const boost::regex reg("[].|*?+(){}^$\\[\\\\]");

	// $& is a perl-like expression that expands to all
	// of the current match
	// The '$' must be prefixed with the escape character '\' for
	// boost to treat it as a literal.
	// Thus, to prefix a matched expression with '\', we use:
	// FIXME: UNICODE
	return from_utf8(boost::regex_replace(to_utf8(expr), reg, "\\\\$&"));
}


vector<docstring> GuiCitation::searchKeys(
	vector<docstring> const & keys_to_search, bool only_keys,
 	docstring const & search_expression, docstring field,
	bool case_sensitive, bool regex)
{
	vector<docstring> foundKeys;

	docstring expr = support::trim(search_expression);
	if (expr.empty())
		return foundKeys;

	if (!regex)
		// We must escape special chars in the search_expr so that
		// it is treated as a simple string by boost::regex.
		expr = escape_special_chars(expr);

	boost::regex reg_exp(to_utf8(expr), case_sensitive ?
		boost::regex_constants::normal : boost::regex_constants::icase);

	vector<docstring>::const_iterator it = keys_to_search.begin();
	vector<docstring>::const_iterator end = keys_to_search.end();
	for (; it != end; ++it ) {
		BiblioInfo::const_iterator info = bibkeysInfo_.find(*it);
		if (info == bibkeysInfo_.end())
			continue;
		
		BibTeXInfo const & kvm = info->second;
		string data;
		if (only_keys)
			data = to_utf8(*it);
		else if (field.empty())
			data = to_utf8(*it) + ' ' + to_utf8(kvm.allData);
		else if (kvm.hasField(field))
			data = to_utf8(kvm.getValueForField(field));
		
		if (data.empty())
			continue;

		try {
			if (boost::regex_search(data, reg_exp))
				foundKeys.push_back(*it);
		}
		catch (boost::regex_error &) {
			return vector<docstring>();
		}
	}
	return foundKeys;
}


Dialog * createGuiCitation(LyXView & lv) { return new GuiCitation(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiCitation_moc.cpp"

