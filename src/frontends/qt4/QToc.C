/**
 * \file QToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QToc.h"
#include "TocModel.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlToc.h"

#include <algorithm>

using std::endl;

using std::pair;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

QToc::QToc(Dialog & parent)
	: ControlToc(parent)
{
	update();
}


bool QToc::canOutline()
{
	vector<string> const & types = getTypes();

	BOOST_ASSERT(type_ >= 0 && type_ < int(types.size()));
	return ControlToc::canOutline(types[type_]);
}


QStandardItemModel * QToc::tocModel()
{
	lyxerr[Debug::GUI]
		<< "QToc: type_ " << type_
		<< "  toc_models_.size() " << toc_models_.size()
		<< endl;

	BOOST_ASSERT(type_ >= 0 && type_ < int(toc_models_.size()));
	return toc_models_[type_];
}


QStandardItemModel * QToc::setTocModel(int type)
{
	type_ = type;

	lyxerr[Debug::GUI]
		<< "QToc: type_ " << type_
		<< "  toc_models_.size() " << toc_models_.size()
		<< endl;

	BOOST_ASSERT(type_ >= 0 && type_ < int(toc_models_.size()));
	return toc_models_[type_];
}


QModelIndex const QToc::getCurrentIndex()
{
	vector<string> const & types = getTypes();
	toc::TocItem const item = getCurrentTocItem(types[type_]);
	if (item.id_ == -1) {
		lyxerr[Debug::GUI]
			<< "QToc::getCurrentIndex(): TocItem is invalid!" << endl;
		return QModelIndex();
	}

	string toc_str = item.str;
	toc_str.erase(0, toc_str.find(' ') + 1);

	return toc_models_[type_]->index(toc_str);
}


void QToc::goTo(QModelIndex const & index)
{
	if (!index.isValid()) {
		lyxerr[Debug::GUI]
			<< "QToc::goTo(): QModelIndex is invalid!"
			<< endl;
		return;
	}
	
	lyxerr[Debug::GUI]
		<< "QToc::goTo " << toc_models_[type_]->item(index).str
		<< endl;

	ControlToc::goTo(toc_models_[type_]->item(index));
}


void QToc::update()
{
	toc_models_.clear();

	QStringList type_list;

	type_ = 0;

	vector<string> const & types = getTypes();
	string const & selected_type = toc::getType(params().getCmdName());
	lyxerr[Debug::GUI] << "selected_type " << selected_type	<< endl;

	QString gui_names_;
	for (size_t i = 0; i != types.size(); ++i) {
		string const & type_str = types[i];
		type_list.append(toqstr(getGuiName(type_str)));
		if (type_str == selected_type)
			type_ = i;
		
		lyxerr[Debug::GUI]
			<< "QToc: new type " << type_str
			<< "\ttoc_models_.size() " << toc_models_.size()
			<< endl;

		toc_models_.push_back(new TocModel(getContents(types[i])));
	}
	type_model_.setStringList(type_list);
}


void QToc::updateToc(int type)
{
	vector<string> const & choice = getTypes();

	toc_models_[type] = new TocModel(getContents(choice[type]));
}


void QToc::move(toc::OutlineOp const operation, QModelIndex & index)
{
	int toc_id = toc_models_[type_]->item(index).id_;
	string toc_str = toc_models_[type_]->item(index).str;
	toc_str.erase(0, toc_str.find(' ') + 1);

	outline(operation);
	updateToc(type_);

	lyxerr[Debug::GUI]
		<< "Toc id " << toc_id
		<< "  Toc str " << toc_str
		<< endl;

	index = toc_models_[type_]->index(toc_str);
}

} // namespace frontend
} // namespace lyx
