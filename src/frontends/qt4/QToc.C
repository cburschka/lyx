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
}


bool QToc::canOutline(int type) const
{
	if (type < 0) 
		return false;

	return ControlToc::canOutline(type);
}


int QToc::getTocDepth(int type)
{
	if (type < 0)
		return 0;
	return toc_models_[type]->modelDepth();
}


QStandardItemModel * QToc::tocModel(int type)
{
	if (type < 0)
		return 0;

	if (toc_models_.empty()) {
		LYXERR(Debug::GUI) << "QToc::tocModel(): no types available " << endl;
		return 0;
	}

	LYXERR(Debug::GUI)
		<< "QToc: type_ " << type
		<< "  toc_models_.size() " << toc_models_.size()
		<< endl;

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));
	return toc_models_[type];
}


QModelIndex const QToc::getCurrentIndex(int type) const
{
	if (type < 0)
		return QModelIndex();

	return toc_models_[type]->modelIndex(getCurrentTocItem(type));
}


void QToc::goTo(int type, QModelIndex const & index)
{
	if (type < 0 || !index.isValid() 
		|| index.model() != toc_models_[type]) {
		LYXERR(Debug::GUI)
			<< "QToc::goTo(): QModelIndex is invalid!"
			<< endl;
		return;
	}

	BOOST_ASSERT(type >= 0 && type < int(toc_models_.size()));

	TocIterator const it = toc_models_[type]->tocIterator(index);
	
	LYXERR(Debug::GUI)
		<< "QToc::goTo " << lyx::to_utf8(it->str())
		<< endl;

	ControlToc::goTo(*it);
}


bool QToc::initialiseParams(std::string const & data)
{
	if (!ControlToc::initialiseParams(data))
		return false;
	update();
	return true;
}


void QToc::update()
{
	updateType();
	updateToc();
	modelReset();
}


void QToc::updateType()
{
	QStringList type_list;

	vector<docstring> const & type_names = typeNames();
	BOOST_ASSERT(!type_names.empty());
	for (size_t i = 0; i != type_names.size(); ++i)
		type_list.append(toqstr(type_names[i]));

	type_model_.setStringList(type_list);
}


void QToc::updateToc()
{
	toc_models_.clear();
	TocList::const_iterator it = tocs().begin();
	TocList::const_iterator end = tocs().end();
	for (; it != end; ++it)
		toc_models_.push_back(new TocModel(it->second));
}


} // namespace frontend
} // namespace lyx

#include "QToc_moc.cpp"
