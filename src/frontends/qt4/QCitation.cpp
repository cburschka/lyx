/*
 * \file QCitation.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QCitation.h"

#include "qt_helpers.h"

#include "support/lstrings.h"
#include "support/docstring.h"

#include "debug.h"

#include <vector>
#include <string>

using std::vector;
using std::string;


template<typename String> static QStringList to_qstring_list(vector<String> const & v)
{
	QStringList qlist;

	for (size_t i = 0; i != v.size(); ++i) {
		if (v[i].empty())
			continue;
		qlist.append(lyx::toqstr(v[i]));
	}
	return qlist;
}


static vector<lyx::docstring> const to_docstring_vector(QStringList const & qlist)
{
	vector<lyx::docstring> v;
	for (int i = 0; i != qlist.size(); ++i) {
		if (qlist[i].isEmpty())
			continue;
		v.push_back(lyx::qstring_to_ucs4(qlist[i]));
	}
	return v;
}


namespace lyx {
namespace frontend {


QCitation::QCitation(Dialog & parent)
	: ControlCitation(parent)
{
}


void QCitation::apply(int const choice, bool const full, bool const force,
		      QString before, QString after)
{
	if (cited_keys_.isEmpty())
		return;

	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	params().setCmdName(command);
	params()["key"] = qstring_to_ucs4(cited_keys_.join(","));
	params()["before"] = qstring_to_ucs4(before);
	params()["after"] = qstring_to_ucs4(after);
	dispatchParams();
}


void QCitation::clearSelection()
{
	cited_keys_.clear();
	selected_model_.setStringList(cited_keys_);
}


QString QCitation::textBefore()
{
	return toqstr(params()["before"]);
}


QString QCitation::textAfter()
{
	return toqstr(params()["after"]);
}


bool QCitation::initialiseParams(std::string const & data)
{
	if (!ControlCitation::initialiseParams(data))
		return false;
	init();
	return true;
}


void QCitation::init()
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


void QCitation::findKey(QString const & str, bool only_keys,
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
			LYXERR(Debug::GUI) << "QCitation::findKey: optimisation disabled!" << std::endl;
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
	
	//First, filter by entryType, which will be faster than 
	//what follows, so we may get to do that on less.
	vector<docstring> keyVector = to_docstring_vector(keys);
	filterByEntryType(keyVector, entryType);
	
	if (str.isEmpty())
		result = to_qstring_list(keyVector);
	else
		result = to_qstring_list(searchKeys(keyVector, only_keys, 
			qstring_to_ucs4(str), field, case_sensitive, reg_exp));
	
	available_model_.setStringList(result);
}


QStringList QCitation::getFieldsAsQStringList() {
	return to_qstring_list(availableFields());
}


QStringList QCitation::getEntriesAsQStringList() {
	return to_qstring_list(availableEntries());
}


void QCitation::addKey(QModelIndex const & index)
{
	cited_keys_.append(index.data().toString());
	selected_model_.setStringList(cited_keys_);
}


void QCitation::deleteKey(QModelIndex const & index)
{
	cited_keys_.removeAt(index.row());
	selected_model_.setStringList(cited_keys_);
}


void QCitation::upKey(QModelIndex const & index)
{
	int pos = index.row();
	cited_keys_.swap(pos, pos - 1);
	selected_model_.setStringList(cited_keys_);
}


void QCitation::downKey(QModelIndex const & index)
{
	int pos = index.row();
	cited_keys_.swap(pos, pos + 1);
	selected_model_.setStringList(cited_keys_);
}


QStringList QCitation::citationStyles(int sel)
{
	docstring const key = qstring_to_ucs4(cited_keys_[sel]);
	return to_qstring_list(getCiteStrings(key));
}


QString QCitation::getKeyInfo(QString const & sel)
{
	return toqstr(getInfo(qstring_to_ucs4(sel)));
}


} // namespace frontend
} // namespace lyx
