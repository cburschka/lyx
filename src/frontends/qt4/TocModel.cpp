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
#include "TocBackend.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"

#include <climits>

using namespace std;

namespace lyx {
namespace frontend {


TocItem const & TocModel::tocItem(QModelIndex const & index) const
{
	return toc_[data(index, Qt::UserRole).toUInt()];
}


QModelIndex TocModel::modelIndex(DocIterator const & dit) const
{
	if (toc_.empty())
		return QModelIndex();

	unsigned int const toc_index = toc_.item(dit) - toc_.begin();

	QModelIndexList list = match(index(0, 0), Qt::UserRole,
		QVariant(toc_index), 1,
		Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

	LASSERT(!list.isEmpty(), return QModelIndex());
	return list[0];
}


TocModel::TocModel(Toc const & toc): toc_(toc)
{
	if (toc_.empty())
		return;
	int current_row;
	QModelIndex top_level_item;
	insertColumns(0, 1);
	maxdepth_ = 0;
	mindepth_ = INT_MAX;

	size_t end = toc.size();
	for (unsigned int index = 0; index != end; ++index) {
		TocItem const & item = toc_[index];
		maxdepth_ = max(maxdepth_, item.depth());
		mindepth_ = min(mindepth_, item.depth());
		current_row = rowCount();
		insertRows(current_row, 1);
		top_level_item = QStandardItemModel::index(current_row, 0);
		setData(top_level_item, toqstr(item.str()), Qt::DisplayRole);
		setData(top_level_item, index, Qt::UserRole);

		LYXERR(Debug::GUI, "Toc: at depth " << item.depth()
			<< ", added item " << item.str());

		populate(index, top_level_item);
		if (index >= end)
			break;
	}

	setHeaderData(0, Qt::Horizontal, QVariant("title"), Qt::DisplayRole);
//	emit headerDataChanged();
}


void TocModel::populate(unsigned int & index, QModelIndex const & parent)
{
	int curdepth = toc_[index].depth() + 1;

	int current_row;
	QModelIndex child_item;
	insertColumns(0, 1, parent);

	size_t end = toc_.size();
	++index;
	for (; index != end; ++index) {
		TocItem const & item = toc_[index];
		if (item.depth() < curdepth) {
			--index;
			return;
		}
		maxdepth_ = max(maxdepth_, item.depth());
		mindepth_ = min(mindepth_, item.depth());
		current_row = rowCount(parent);
		insertRows(current_row, 1, parent);
		child_item = QStandardItemModel::index(current_row, 0, parent);
		setData(child_item, toqstr(item.str()), Qt::DisplayRole);
		setData(child_item, index, Qt::UserRole);
		populate(index, child_item);
		if (index >= end)
			break;
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
	return models_[type]->modelIndex(bv_->cursor());
}


void TocModels::goTo(int type, QModelIndex const & index) const
{
	if (type < 0 || !index.isValid()
		|| index.model() != models_[type]) {
		LYXERR(Debug::GUI, "TocModels::goTo(): QModelIndex is invalid!");
		return;
	}

	LASSERT(type >= 0 && type < int(models_.size()), /**/);
	TocItem const item = models_[type]->tocItem(index);
	LYXERR(Debug::GUI, "TocModels::goTo " << item.str());
	dispatch(item.action());
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
