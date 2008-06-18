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

#include <QSortFilterProxyModel>

#include <climits>

using namespace std;

namespace lyx {
namespace frontend {

TocTypeModel::TocTypeModel(QObject * parent): QStandardItemModel(parent)
{
}


void TocTypeModel::reset()
{
	QStandardItemModel::reset();
}


TocItem const & TocModel::tocItem(QModelIndex const & index) const
{
	return (*toc_)[data(index, Qt::UserRole).toUInt()];
}


QModelIndex TocModel::modelIndex(DocIterator const & dit) const
{
	if (toc_->empty())
		return QModelIndex();

	unsigned int const toc_index = toc_->item(dit) - toc_->begin();

	QModelIndexList list = match(index(0, 0), Qt::UserRole,
		QVariant(toc_index), 1,
		Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

	LASSERT(!list.isEmpty(), return QModelIndex());
	return list[0];
}


TocModel::TocModel(QObject * parent): QStandardItemModel(parent)
{
}


void TocModel::reset()
{
	QStandardItemModel::reset();
}


void TocModel::reset(Toc const & toc)
{
	toc_ = &toc;
	if (toc_->empty()) {
		reset();
		return;
	}

	blockSignals(true);
	int current_row;
	QModelIndex top_level_item;
	insertColumns(0, 1);
	maxdepth_ = 0;
	mindepth_ = INT_MAX;

	size_t end = toc_->size();
	for (unsigned int index = 0; index != end; ++index) {
		TocItem const & item = (*toc_)[index];
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
	blockSignals(false);
	reset();
//	emit headerDataChanged();
}


void TocModel::populate(unsigned int & index, QModelIndex const & parent)
{
	int curdepth = (*toc_)[index].depth() + 1;

	int current_row;
	QModelIndex child_item;
	insertColumns(0, 1, parent);

	size_t end = toc_->size();
	++index;
	for (; index != end; ++index) {
		TocItem const & item = (*toc_)[index];
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

TocModels::TocModels(): bv_(0)
{
	names_ = new TocTypeModel(this);
	names_sorted_ = new QSortFilterProxyModel(this);
	names_sorted_->setSourceModel(names_);
#if QT_VERSION >= 0x040300
	names_sorted_->setSortLocaleAware(true);
#endif
	names_sorted_->sort(0);
}


void TocModels::clear()	
{
	names_->blockSignals(true);
	names_->clear();
	names_->blockSignals(false);
	iterator end = models_.end();
	for (iterator it = models_.begin(); it != end;  ++it) {
		it.value()->blockSignals(true);
		it.value()->clear();
		it.value()->blockSignals(false);
	}
}


int TocModels::depth(QString const & type)
{
	const_iterator it = models_.find(type);
	if (!bv_ || it == models_.end())
		return 0;
	return it.value()->modelDepth();
}


QStandardItemModel * TocModels::model(QString const & type)
{
	if (!bv_)
		return 0;
	iterator it = models_.find(type);
	if (it != models_.end())
		return it.value();
	LYXERR0("type not found: " << type);
	return 0;
}


QAbstractItemModel * TocModels::nameModel()
{
	return names_sorted_;
}


QModelIndex TocModels::currentIndex(QString const & type) const
{
	const_iterator it = models_.find(type);
	if (!bv_ || it == models_.end())
		return QModelIndex();
	return it.value()->modelIndex(bv_->cursor());
}


void TocModels::goTo(QString const & type, QModelIndex const & index) const
{
	const_iterator it = models_.find(type);
	if (it == models_.end() || !index.isValid()) {
		LYXERR(Debug::GUI, "TocModels::goTo(): QModelIndex is invalid!");
		return;
	}
	LASSERT(index.model() == it.value(), return);
	TocItem const item = it.value()->tocItem(index);
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
	if (!bv_) {
		iterator end = models_.end();
		for (iterator it = models_.begin(); it != end;  ++it)
			it.value()->reset();
		names_->reset();
		return;
	}

	names_->blockSignals(true);
	names_->insertColumns(0, 1);
	TocList const & tocs = bv_->buffer().masterBuffer()->tocBackend().tocs();
	TocList::const_iterator it = tocs.begin();
	TocList::const_iterator toc_end = tocs.end();
	for (; it != toc_end; ++it) {
		QString const type = toqstr(it->first);

		// First, fill in the toc models.
		iterator mod_it = models_.find(type);
		if (mod_it == models_.end())
			mod_it = models_.insert(type, new TocModel(this));
		mod_it.value()->reset(it->second);

		// Fill in the names_ model.
		QString const gui_name = guiName(it->first, bv->buffer().params());
		int const current_row = names_->rowCount();
		names_->insertRows(current_row, 1);
		QModelIndex const index = names_->index(current_row, 0);
		names_->setData(index, gui_name, Qt::DisplayRole);
		names_->setData(index, type, Qt::UserRole);
	}
	names_->blockSignals(false);
	names_->reset();
}


} // namespace frontend
} // namespace lyx

#include "TocModel_moc.cpp"
