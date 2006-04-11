/**
 * \file QCitation.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCitation.h"
#include "QCitation.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "bufferparams.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlCitation.h"

#include "support/lstrings.h"

#include <vector>
#include <string>
#include <iostream>
using std::cout;
using std::endl;

using std::vector;
using std::string;


QStringList toQStringList(vector<string> const & v)
{
	QStringList qlist;

	for (size_t i=0; i != v.size(); ++i) {
		if (v[i].empty())
			continue;
		qlist.append(toqstr(v[i]));
	}
	return qlist;
}


void toVector(vector<string> & v, const QStringList & qlist)
{
	v.clear();

	for (size_t i=0; i != qlist.size(); ++i)
		v.push_back(fromqstr(qlist[i]));
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
//	InsetCommandParams & params = params();

	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	params().setContents(fromqstr(selected_keys_.stringList().join(",")));
	params().setSecOptions(fromqstr(before));
	params().setOptions(fromqstr(after));
	dispatchParams();

/*
	if (dialog().controller().isBufferDependent()) {
		if (!dialog().kernel().isBufferAvailable() ||
		    dialog().kernel().isBufferReadonly())
			return;
	}
	dialog().controller().dispatchParams();

	if (dialog().controller().disconnectOnApply()) {
		dialog().kernel().disconnect(name());
		dialog().controller().initialiseParams(string());
		dialog().view().update();
	}
*/
}


QString QCitation::textBefore()
{
	return toqstr(params().getSecOptions());
}


QString QCitation::textAfter()
{
	return toqstr(params().getOptions());
}


void QCitation::updateModel()
{
	// Make the list of all available bibliography keys
	QStringList keys = toQStringList(biblio::getKeys(bibkeysInfo()));
	available_keys_.setStringList(keys);

	// Ditto for the keys cited in this inset
	QString str = toqstr(params().getContents());
	if (!str.isEmpty()) {
		keys = str.split(",");
		selected_keys_.setStringList(keys);
	}
}


bool QCitation::isValid()
{
	return selected_keys_.rowCount() > 0;
}


QModelIndex QCitation::findKey(QString const & str, QModelIndex const & index) const
{
	QStringList const avail = available_keys_.stringList();
	int const pos = avail.indexOf(str, index.row());
	if (pos == -1)
		return index;
	return available_keys_.index(pos);
}


QModelIndex QCitation::findKey(QString const & str) const
{
	cout << "Find text " << fromqstr(str) << endl;

	QStringList const avail = available_keys_.stringList();
	QRegExp reg_exp(str);

	int const pos = avail.indexOf(reg_exp);
	if (pos == -1)
		return QModelIndex();

	cout << "found key " << fromqstr(avail[pos]) << " at pos " << pos << endl;
	return available_keys_.index(pos);
}


void QCitation::addKeys(QModelIndexList const & indexes)
{
	QModelIndex index;

	if (indexes.empty())
		return;

	QStringList keys = selected_keys_.stringList();

	foreach(index, indexes) {
		if (keys.indexOf(index.data().toString()) == -1)
			keys.append(index.data().toString());
	}

	selected_keys_.setStringList(keys);
}


void QCitation::deleteKeys(QModelIndexList const & indexes)
{
	QModelIndex index;

	if (indexes.empty())
		return;

	QStringList keys = selected_keys_.stringList();

	foreach(index, indexes) {
		int const pos = keys.indexOf(index.data().toString());
		if (pos != -1)
			keys.removeAt(pos);
	}

	selected_keys_.setStringList(keys);
}


void QCitation::upKey(QModelIndexList const & indexes)
{
	if (indexes.empty() || indexes.size() > 1)
		return;

	int pos = indexes[0].row();
	if (pos < 1)
		return;

	QStringList keys = selected_keys_.stringList();
	keys.swap(pos, pos-1);
	selected_keys_.setStringList(keys);
}


void QCitation::downKey(QModelIndexList const & indexes)
{
	if (indexes.empty() || indexes.size() > 1)
		return;

	int pos = indexes[0].row();
	if (pos >= selected_keys_.rowCount() - 1)
		return;

	QStringList keys = selected_keys_.stringList();
	keys.swap(pos, pos+1);
	selected_keys_.setStringList(keys);
}


QStringList QCitation::citationStyles(int sel)
{
	string key = fromqstr(selected_keys_.stringList()[sel]);

	return toQStringList(getCiteStrings(key));
}

} // namespace frontend
} // namespace lyx
