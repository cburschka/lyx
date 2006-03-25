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

#include "QCitation.h"
#include "QCitationDialog.h"
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

void toQStringList(QStringList & qlist, vector<string> const & v)
{
	qlist.clear();
	for (size_t i=0; i != v.size(); ++i) {
		if (v[i].empty())
			continue;
		qlist.append(toqstr(v[i]));
	}
}

void toVector(vector<string> & v, const QStringList & qlist)
{
	v.clear();

	for (size_t i=0; i != qlist.size(); ++i)
		v.push_back(fromqstr(qlist[i]));
}

namespace lyx {
namespace frontend {

typedef QController<ControlCitation, QView<QCitationDialog> > base_class;


QCitation::QCitation(Dialog & parent)
	: base_class(parent, _("Citation"))
{
}


void QCitation::apply()
{
	InsetCommandParams & params = controller().params();
	dialog_->update(params);

	params.setContents(fromqstr(selected_keys_.stringList().join("'")));
}


void QCitation::build_dialog()
{
	dialog_.reset(new QCitationDialog(this));
}


void QCitation::update_contents()
{
	QStringList keys;
	
	// Make the list of all available bibliography keys
	toQStringList(keys,
		biblio::getKeys(controller().bibkeysInfo()));
	available_keys_.setStringList(keys);

	// Ditto for the keys cited in this inset
	QString str = toqstr(controller().params().getContents());
	if (!str.isEmpty()) {
		keys = str.split(",");
		selected_keys_.setStringList(keys);
	}

	dialog_->update(controller().params());

	bc().valid(isValid());
}

void QCitation::hide()
{
	QDialogView::hide();
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
	QStringList const avail = available_keys_.stringList();
	int const pos = avail.indexOf(str);
	if (pos == -1)
		return QModelIndex();
	return available_keys_.index(pos);
}

void QCitation::addKeys(QModelIndexList const & indexes)
{
	// = selectionModel->selectedIndexes();

	QModelIndex index;

	if (indexes.empty())
		return;

	QStringList keys = selected_keys_.stringList();
	
	foreach(index, indexes) {
		if (keys.indexOf(index.data().toString()) == -1)
			keys.append(index.data().toString());
	}

	selected_keys_.setStringList(keys);
	
	changed();
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
	
	changed();
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
	
	changed();
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
	
	changed();
}






} // namespace frontend
} // namespace lyx
