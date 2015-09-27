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
#include "LyX.h"
#include "qt_helpers.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/lassert.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>


#include <climits>

using namespace std;

namespace lyx {
namespace frontend {

/// A QStandardItemModel that gives access to the reset methods.
/// This is needed in order to fix http://www.lyx.org/trac/ticket/3740
class TocTypeModel : public QStandardItemModel
{
public:
	///
	TocTypeModel(QObject * parent) : QStandardItemModel(parent)
	{}
	///
	void reset()
	{
#if (QT_VERSION < 0x050000)
		QStandardItemModel::reset();
#else
		QStandardItemModel::endResetModel();
#endif
	}
	///
	void beginResetModel()
	{ 
	#if QT_VERSION >= 0x040600
		QStandardItemModel::beginResetModel(); 
	#endif
	}
	///
	void endResetModel()
	{ 
	#if QT_VERSION >= 0x040600
		QStandardItemModel::endResetModel(); 
	#else
		QStandardItemModel::reset();
	#endif
	}
};


///////////////////////////////////////////////////////////////////////////////
//
// TocModel
//
///////////////////////////////////////////////////////////////////////////////

TocModel::TocModel(QObject * parent)
	: model_(new TocTypeModel(parent)),
	  sorted_model_(new QSortFilterProxyModel(parent)),
	  is_sorted_(false), toc_(make_shared<Toc const>()),
	  maxdepth_(0), mindepth_(0)
{
	sorted_model_->setSortLocaleAware(true);
	sorted_model_->setSourceModel(model_);
}


QAbstractItemModel * TocModel::model()
{
	if (is_sorted_)
		return sorted_model_;
	return model_;
}


QAbstractItemModel const * TocModel::model() const
{
	if (is_sorted_)
		return sorted_model_;
	return model_;
}


void TocModel::clear()
{
	model_->blockSignals(true);
	model_->clear();
	toc_ = make_shared<Toc const>();
	model_->blockSignals(false);
}


void TocModel::sort(bool sort_it)
{
	is_sorted_ = sort_it;
	if (is_sorted_)
		sorted_model_->sort(0);
}


TocItem const & TocModel::tocItem(QModelIndex const & index) const
{
	return (*toc_)[model()->data(index, Qt::UserRole).toUInt()];
}


QModelIndex TocModel::modelIndex(DocIterator const & dit) const
{
	if (toc_->empty())
		return QModelIndex();

	unsigned int const toc_index = toc_->item(dit) - toc_->begin();

	QModelIndexList list = model()->match(model()->index(0, 0), Qt::UserRole,
		QVariant(toc_index), 1,
		Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

	LASSERT(!list.isEmpty(), return QModelIndex());
	return list[0];
}


void TocModel::reset()
{
	model_->reset();
}


void TocModel::updateItem(DocIterator const & dit)
{
	QModelIndex index = modelIndex(dit);
	TocItem const & toc_item = tocItem(index);
	model_->setData(index, toqstr(toc_item.asString()), Qt::DisplayRole);
	model_->setData(index, toqstr(toc_item.tooltip()), Qt::ToolTipRole);
}


void TocModel::reset(shared_ptr<Toc const> toc)
{
	toc_ = toc;
	if (toc_->empty()) {
		maxdepth_ = 0;
		mindepth_ = 0;
		reset();
		return;
	}

	model_->blockSignals(true);
	model_->beginResetModel();
	model_->insertColumns(0, 1);
	maxdepth_ = 0;
	mindepth_ = INT_MAX;

	size_t end = toc_->size();
	for (unsigned int index = 0; index != end; ++index) {
		TocItem const & item = (*toc_)[index];
		maxdepth_ = max(maxdepth_, item.depth());
		mindepth_ = min(mindepth_, item.depth());
		int current_row = model_->rowCount();
		model_->insertRows(current_row, 1);
		QModelIndex top_level_item = model_->index(current_row, 0);
		model_->setData(top_level_item, toqstr(item.asString()), Qt::DisplayRole);
		model_->setData(top_level_item, index, Qt::UserRole);
		model_->setData(top_level_item, toqstr(item.tooltip()), Qt::ToolTipRole);

		LYXERR(Debug::GUI, "Toc: at depth " << item.depth()
			<< ", added item " << item.asString());

		populate(index, top_level_item);
		if (index >= end)
			break;
	}

	model_->setHeaderData(0, Qt::Horizontal, QVariant("title"), Qt::DisplayRole);
	sorted_model_->setSourceModel(model_);
	if (is_sorted_)
		sorted_model_->sort(0);
	model_->blockSignals(false);
	model_->endResetModel();
}


void TocModel::populate(unsigned int & index, QModelIndex const & parent)
{
	int curdepth = (*toc_)[index].depth() + 1;

	QModelIndex child_item;
	model_->insertColumns(0, 1, parent);

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
		int current_row = model_->rowCount(parent);
		model_->insertRows(current_row, 1, parent);
		child_item = model_->index(current_row, 0, parent);
		model_->setData(child_item, toqstr(item.asString()), Qt::DisplayRole);
		model_->setData(child_item, index, Qt::UserRole);
		model_->setData(child_item, toqstr(item.tooltip()), Qt::ToolTipRole);
		populate(index, child_item);
		if (index >= end)
			break;
	}
}


int TocModel::modelDepth() const
{
	int const d = maxdepth_ - mindepth_;
	LASSERT(d >= 0 && d <= 100, return 0);
	return d;
}


///////////////////////////////////////////////////////////////////////////////
//
// TocModels
//
///////////////////////////////////////////////////////////////////////////////

TocModels::TocModels()
	: bv_(0)
{
	names_ = new TocTypeModel(this);
	names_sorted_ = new TocModelSortProxyModel(this);
	names_sorted_->setSourceModel(names_);
	names_sorted_->setSortLocaleAware(true);
	names_sorted_->sort(0);
}


void TocModels::clear()
{
	names_->blockSignals(true);
	names_->clear();
	names_->blockSignals(false);
	iterator end = models_.end();
	for (iterator it = models_.begin(); it != end;  ++it)
		it.value()->clear();
}


int TocModels::depth(QString const & type)
{
	const_iterator it = models_.find(type);
	if (!bv_ || it == models_.end())
		return 0;
	return it.value()->modelDepth();
}


QAbstractItemModel * TocModels::model(QString const & type)
{
	if (!bv_)
		return 0;
	iterator it = models_.find(type);
	if (it != models_.end())
		return it.value()->model();
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
	LASSERT(index.model() == it.value()->model(), return);
	TocItem const item = it.value()->tocItem(index);
	LYXERR(Debug::GUI, "TocModels::goTo " << item.asString());
	dispatch(item.action());
}


TocItem const TocModels::currentItem(QString const & type,
	QModelIndex const & index) const
{
	const_iterator it = models_.find(type);
	if (it == models_.end() || !index.isValid()) {
		LYXERR(Debug::GUI, "TocModels::currentItem(): QModelIndex is invalid!");
		return TocItem();
	}
	LASSERT(index.model() == it.value()->model(), return TocItem());
	
	return it.value()->tocItem(index);
}
 

void TocModels::updateItem(QString const & type, DocIterator const & dit)
{
	models_[type]->updateItem(dit);
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
	names_->beginResetModel();
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
	names_->endResetModel();
}


bool TocModels::isSorted(QString const & type) const
{
	const_iterator it = models_.find(type);
	if (it == models_.end()) {
		LYXERR0("type not found: " << type);
		return false;
	}
	return it.value()->isSorted();
}


void TocModels::sort(QString const & type, bool sort_it)
{
	iterator it = models_.find(type);
	if (it == models_.end())
		LYXERR0("type not found: " << type);
	else
		it.value()->sort(sort_it);
}

} // namespace frontend
} // namespace lyx

#include "moc_TocModel.cpp"
