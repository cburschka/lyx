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

} // namespace frontend
} // namespace lyx

#include "TocModel_moc.cpp"
