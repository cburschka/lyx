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

	if (types.empty())
		return false;

	BOOST_ASSERT(type_ >= 0 && type_ < int(types.size()));
	return ControlToc::canOutline(types[type_]);
}


QStandardItemModel * QToc::tocModel()
{
	lyxerr[Debug::GUI]
		<< "QToc: type_ " << type_
		<< "  toc_models_.size() " << toc_models_.size()
		<< endl;

	if (toc_models_.empty())
		return 0;

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
	TocIterator const it = getCurrentTocItem(types[type_]);
	if (!it->isValid()) {
		lyxerr[Debug::GUI] << "QToc::getCurrentIndex(): TocItem is invalid!" << endl;
		return QModelIndex();
	}

	return toc_models_[type_]->modelIndex(it);
}


void QToc::goTo(QModelIndex const & index)
{
	if (!index.isValid()) {
		lyxerr[Debug::GUI]
			<< "QToc::goTo(): QModelIndex is invalid!"
			<< endl;
		return;
	}

	TocIterator const it = toc_models_[type_]->tocIterator(index);
	
	lyxerr[Debug::GUI]
		<< "QToc::goTo " << lyx::to_utf8(it->str())
		<< endl;

	it->goTo(kernel().lyxview());
}


void QToc::update()
{
	toc_models_.clear();

	QStringList type_list;

	type_ = 0;

	vector<string> const & types = getTypes();
	if (types.empty()) {
		type_model_.setStringList(type_list);
		toc_models_.clear();
		return;
	}

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
	toc_models_[type] = new TocModel(getContents(getTypes()[type]));
}


} // namespace frontend
} // namespace lyx
