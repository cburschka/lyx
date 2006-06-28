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


namespace lyx {
namespace frontend {


QCitation::QCitation(Dialog & parent)
	: ControlCitation(parent)
{
}


void QCitation::apply(int const choice, bool const full, bool const force,
					  QString before, QString after)
{
	if (!isValid())
		return;

	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	string const command =
		biblio::CitationStyle(styles[choice], full, force)
		.asLatexStr();

	params().setCmdName(command);
	params().setContents(fromqstr(selected_keys_.stringList().join(",")));
	params().setSecOptions(fromqstr(before));
	params().setOptions(fromqstr(after));
	dispatchParams();
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


void QCitation::findKey(QString const & str)
{
	QStringList sl = available_keys_.stringList().filter(str, Qt::CaseInsensitive);
	found_keys_.setStringList(sl);
}


void QCitation::addKey(QModelIndex const & index)
{
	QStringList keys = selected_keys_.stringList();
	keys.append(index.data().toString());
	selected_keys_.setStringList(keys);
}


void QCitation::deleteKey(QModelIndex const & index)
{
	QStringList keys = selected_keys_.stringList();
	keys.removeAt(index.row());
	selected_keys_.setStringList(keys);
}


void QCitation::upKey(QModelIndex const & index)
{
	QStringList keys = selected_keys_.stringList();
	int pos = index.row();
	keys.swap(pos, pos - 1);
	selected_keys_.setStringList(keys);
}


void QCitation::downKey(QModelIndex const & index)
{
	QStringList keys = selected_keys_.stringList();
	int pos = index.row();
	keys.swap(pos, pos + 1);
	selected_keys_.setStringList(keys);
}


QStringList QCitation::citationStyles(int sel)
{
	string key = fromqstr(selected_keys_.stringList()[sel]);
	return toQStringList(getCiteStrings(key));
}


QString QCitation::getKeyInfo(QString const & sel)
{
	if (!bibkeysInfo().empty())
		return toqstr(biblio::getInfo(bibkeysInfo(), fromqstr(sel) ));

	return QString();
}


} // namespace frontend
} // namespace lyx
