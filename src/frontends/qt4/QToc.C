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


int QToc::getTocDepth()
{
	return toc_models_[type_]->modelDepth();
}


QStandardItemModel * QToc::tocModel()
{
	if (toc_models_.empty()) {
		lyxerr[Debug::GUI] << "QToc::tocModel(): no types available " << endl;
		return 0;
	}

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
	TocIterator const it = getCurrentTocItem(types[type_]);
	if (it == getContents(types[type_]).end() || !it->isValid()) {
		lyxerr[Debug::GUI] << "QToc::getCurrentIndex(): TocItem is invalid!" << endl;
		return QModelIndex();
	}

	return toc_models_[type_]->modelIndex(it);
}


void QToc::goTo(QModelIndex const & index)
{
	if (!index.isValid() || index.model() != tocModel()) {
		lyxerr[Debug::GUI]
			<< "QToc::goTo(): QModelIndex is invalid!"
			<< endl;
		return;
	}

	TocIterator const it = toc_models_[type_]->tocIterator(index);
	
	lyxerr[Debug::GUI]
		<< "QToc::goTo " << lyx::to_utf8(it->str())
		<< endl;

	ControlToc::goTo(*it);
}


int QToc::getType()
{
	return type_;
}


void QToc::update()
{
	updateType();
	updateToc();
}


void QToc::updateType()
{

	QStringList type_list;

	vector<string> const & types = getTypes();
	if (types.empty()) {
		type_model_.setStringList(type_list);
		toc_models_.clear();
		lyxerr[Debug::GUI] << "QToc::updateType(): no types available " << endl;
		return;
	}

	string selected_type ;
	if(params()["type"].empty()) //Then plain toc...
		selected_type = params().getCmdName();
	else
		selected_type = to_ascii(params()["type"]);

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

	}
	type_model_.setStringList(type_list);
}


void QToc::updateToc()
{
	toc_models_.clear();
	vector<string> const & types = getTypes();

	for (size_t i = 0; i != types.size(); ++i) {

		toc_models_.push_back(new TocModel(getContents(types[i])));
	}
	
}


} // namespace frontend
} // namespace lyx
