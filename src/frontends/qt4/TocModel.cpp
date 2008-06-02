/**
 * \file TocModel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TocModel.h"

#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "DocIterator.h"
#include "FuncRequest.h"
#include "LyXFunc.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"

#include <climits>

using namespace std;

namespace lyx {
namespace frontend {

typedef std::pair<QModelIndex, TocIterator> TocPair;


TocIterator TocModel::tocIterator(QModelIndex const & index) const
{
	TocMap::const_iterator map_it = toc_map_.find(index);
	LASSERT(map_it != toc_map_.end(), /**/);
	return map_it->second;
}


QModelIndex TocModel::modelIndex(TocIterator const & it) const
{
	ModelMap::const_iterator map_it = model_map_.find(it);
	//LASSERT(it != model_map_.end(), /**/);

	if (map_it == model_map_.end())
		return QModelIndex();

	return map_it->second;
}


void TocModel::clear()
{
	QStandardItemModel::clear();
	toc_map_.clear();
	model_map_.clear();
	removeRows(0, rowCount());
	removeColumns(0, columnCount());
}


void TocModel::populate(Toc const & toc)
{
	clear();

	if (toc.empty())
		return;
	int current_row;
	QModelIndex top_level_item;

	TocIterator iter = toc.begin();
	TocIterator end = toc.end();

	insertColumns(0, 1);
	maxdepth_ = 0;
	mindepth_ = INT_MAX;

	while (iter != end) {
		maxdepth_ = max(maxdepth_, iter->depth());
		mindepth_ = min(mindepth_, iter->depth());
		current_row = rowCount();
		insertRows(current_row, 1);
		top_level_item = QStandardItemModel::index(current_row, 0);
		//setData(top_level_item, toqstr(iter->str()));
		setData(top_level_item, toqstr(iter->str()), Qt::DisplayRole);

		// This looks like a gcc bug, in principle this should work:
		//toc_map_[top_level_item] = iter;
		// but it crashes with gcc-4.1 and 4.0.2
		toc_map_.insert( TocPair(top_level_item, iter) );
		model_map_[iter] = top_level_item;

		LYXERR(Debug::GUI, "Toc: at depth " << iter->depth()
			<< ", added item " << toqstr(iter->str()));

		populate(iter, end, top_level_item);

		if (iter == end)
			break;

		++iter;
	}

	setHeaderData(0, Qt::Horizontal, QVariant("title"), Qt::DisplayRole);
//	emit headerDataChanged();
}


void TocModel::populate(TocIterator & iter, TocIterator const & end,
	QModelIndex const & parent)
{
	int curdepth = iter->depth() + 1;

	int current_row;
	QModelIndex child_item;
	insertColumns(0, 1, parent);

	while (iter != end) {
		++iter;

		if (iter == end)
			break;

		if (iter->depth() < curdepth) {
			--iter;
			return;
		}

		maxdepth_ = max(maxdepth_, iter->depth());
		mindepth_ = min(mindepth_, iter->depth());
		current_row = rowCount(parent);
		insertRows(current_row, 1, parent);
		child_item = QStandardItemModel::index(current_row, 0, parent);
		//setData(child_item, toqstr(iter->str()));
		setData(child_item, toqstr(iter->str()), Qt::DisplayRole);

		// This looks like a gcc bug, in principle this should work:
		//toc_map_[child_item] = iter;
		// but it crashes with gcc-4.1 and 4.0.2
		toc_map_.insert( TocPair(child_item, iter) );
		model_map_[iter] = child_item;
		populate(iter, end, child_item);
	}
}


int TocModel::modelDepth() const
{
	return maxdepth_ - mindepth_;
}


///////////////////////////////////////////////////////////////////////////////
// TocModels implementation.
///////////////////////////////////////////////////////////////////////////////
void TocModels::clear()	
{
	types_.clear();
	type_names_.clear();
	const unsigned int size = models_.size();
	for (unsigned int i = 0; i < size; ++i) {
		delete models_[i];
	}
	models_.clear();
}


int TocModels::depth(int type)
{
	if (type < 0)
		return 0;
	return models_[type]->modelDepth();
}


QStandardItemModel * TocModels::model(int type)
{
	if (type < 0)
		return 0;

	if (models_.empty()) {
		LYXERR(Debug::GUI, "TocModels::tocModel(): no types available ");
		return 0;
	}

	LYXERR(Debug::GUI, "TocModels: type " << type
		<< "  models_.size() " << models_.size());

	LASSERT(type >= 0 && type < int(models_.size()), /**/);
	return models_[type];
}


QModelIndex TocModels::currentIndex(int type) const
{
	if (type < 0 || !bv_)
		return QModelIndex();

	TocIterator const it = bv_->buffer().masterBuffer()->tocBackend().item(
		fromqstr(types_[type]), bv_->cursor());
	return models_[type]->modelIndex(it);
}


void TocModels::goTo(int type, QModelIndex const & index) const
{
	if (type < 0 || !index.isValid()
		|| index.model() != models_[type]) {
		LYXERR(Debug::GUI, "TocModels::goTo(): QModelIndex is invalid!");
		return;
	}

	LASSERT(type >= 0 && type < int(models_.size()), /**/);
	TocIterator const it = models_[type]->tocIterator(index);
	LYXERR(Debug::GUI, "TocModels::goTo " << it->str());
	dispatch(it->action());
}


void TocModels::updateBackend() const
{
	bv_->buffer().masterBuffer()->tocBackend().update();
	bv_->buffer().structureChanged();
}


void TocModels::reset(BufferView const * bv)
{
	bv_ = bv;
	clear();
	if (!bv_)
		return;

	TocList const & tocs = bv_->buffer().masterBuffer()->tocBackend().tocs();
	TocList::const_iterator it = tocs.begin();
	TocList::const_iterator end = tocs.end();
	for (; it != end; ++it) {
		types_.push_back(toqstr(it->first));
		type_names_.push_back(guiName(it->first, bv->buffer().params()));
		models_.push_back(new TocModel(it->second));
	}
}


bool TocModels::canOutline(int type) const
{
	if (type < 0 || type >= types_.size())
		return false;
	return types_[type] == "tableofcontents";
}


int TocModels::decodeType(QString const & str) const
{
	QString new_type;
	if (str.contains("tableofcontents")) {
		new_type = "tableofcontents";
	} else if (str.contains("floatlist")) {
		if (str.contains("\"figure"))
			new_type = "figure";
		else if (str.contains("\"table"))
			new_type = "table";
		else if (str.contains("\"algorithm"))
			new_type = "algorithm";
	} else if (!str.isEmpty()) {
		new_type = str;
	} else {
		// Default to Outliner.
		new_type = "tableofcontents";
	}
	int const type = types_.indexOf(new_type);
	if (type != -1)
		return type;
	// If everything else fails, settle on the table of contents which is
	// guaranted to exist.
	return types_.indexOf("tableofcontents");
}

} // namespace frontend
} // namespace lyx

#include "TocModel_moc.cpp"
