/**
 * \file QTocDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include "TocModel.h"

#include "debug.h"

#include <vector>
#include <string>

using std::endl;
using std::pair;
using std::map;
using std::vector;
using std::string;
using std::make_pair;

namespace lyx {
namespace frontend {


TocModel::TocModel(toc::Toc const & toc_list)
{
	populate(toc_list);
}


TocModel const & TocModel::operator=(toc::Toc const & toc_list)
{
	populate(toc_list);
	return *this;
}

toc::TocItem const TocModel::item(QModelIndex const & index) const
{
	ItemMap::const_iterator it = item_map_.find(index);
	BOOST_ASSERT(it != item_map_.end());
	
	return it->second;
}

QModelIndex const TocModel::index(string const & toc_str) const
{
	IndexMap::const_iterator it = index_map_.find(toc_str);
	//BOOST_ASSERT(it != index_map_.end());

	if (it == index_map_.end())
		return QModelIndex();
	
	return it->second;
}

void TocModel::clear()
{
	QStandardItemModel::clear();
	item_map_.clear();
	index_map_.clear();
	removeRows(0, rowCount());
	removeColumns(0, columnCount());
}


void TocModel::populate(toc::Toc const & toc_list)
{
	clear();

	if (toc_list.empty())
		return;

	int current_row;
	QModelIndex top_level_item;

	toc::Toc::const_iterator iter = toc_list.begin();
	toc::Toc::const_iterator end = toc_list.end();

    insertColumns(0, 1);

	while (iter != end) {

		if (iter->depth == 1) {

			current_row = rowCount();
			insertRows(current_row, 1);
			top_level_item = QStandardItemModel::index(current_row, 0);
			//setData(top_level_item, toqstr(iter->str));
			setData(top_level_item, toqstr(iter->str), Qt::DisplayRole);
			item_map_.insert(make_pair(top_level_item, *iter));
			index_map_.insert(make_pair(
				iter->str.substr(iter->str.find(' ') + 1), top_level_item));

			lyxerr[Debug::GUI]
				<< "Toc: at depth " << iter->depth
				<< ", added item " << iter->str
				<< endl;

			populate(iter, end, top_level_item);
		}

		if (iter == end)
			break;

		++iter;
	}
	
	setHeaderData(0, Qt::Horizontal, QVariant("title"), Qt::DisplayRole);
//	emit headerDataChanged();
}


void TocModel::populate(toc::Toc::const_iterator & iter,
						toc::Toc::const_iterator const & end,
						QModelIndex const & parent)
{
	int curdepth = iter->depth + 1;
	int current_row;
	QModelIndex child_item;

    insertColumns(0, 1, parent);
	while (iter != end) {

		++iter;

		if (iter == end)
			break;

		if (iter->depth < curdepth) {
			--iter;
			return;
		}
		if (iter->depth > curdepth) {
			return;
		}
		
		current_row = rowCount(parent);
		insertRows(current_row, 1, parent);
		child_item = QStandardItemModel::index(current_row, 0, parent);
		//setData(child_item, toqstr(iter->str));
		setData(child_item, toqstr(iter->str), Qt::DisplayRole);
		item_map_.insert(make_pair(child_item, *iter));
		index_map_.insert(make_pair(
			iter->str.substr(iter->str.find(' ') + 1), child_item));

//		lyxerr[Debug::GUI]
//			<< "Toc: at depth " << iter->depth
//			<< ", added item " << iter->str 
//			<< endl;

		populate(iter, end, child_item);
	}
}

} // namespace frontend
} // namespace lyx
