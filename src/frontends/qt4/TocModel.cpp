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
// FIXME: Better appropriately subclass QStandardItemModel and implement
// the toc-specific reset methods there.
class TocTypeModel : public QStandardItemModel
{
public:
	///
	TocTypeModel(QObject * parent) : QStandardItemModel(parent)
	{}
	///
	void reset()
	{
		QStandardItemModel::beginResetModel();
		QStandardItemModel::endResetModel();
	}
	///
	void beginResetModel()
	{
		QStandardItemModel::beginResetModel();
	}
	///
	void endResetModel()
	{
		QStandardItemModel::endResetModel();
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
	  is_sorted_(false), toc_(new Toc()),
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
	toc_ = make_shared<Toc>();
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

	unsigned int const toc_index = TocBackend::findItem(*toc_, dit) -
	                               toc_->begin();

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


void TocModel::setString(TocItem const & item, QModelIndex index)
{
	// Use implicit sharing of QStrings
	QString str = toqstr(item.asString());
	model_->setData(index, str, Qt::DisplayRole);
	model_->setData(index, str, Qt::ToolTipRole);
}


void TocModel::updateItem(DocIterator const & dit)
{
	QModelIndex const index = modelIndex(dit);
	setString(tocItem(index), index);
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
		setString(item, top_level_item);
		model_->setData(top_level_item, index, Qt::UserRole);

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
		setString(item, child_item);
		model_->setData(child_item, index, Qt::UserRole);
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
	if (it == models_.end())
		return 0;
	return it.value()->modelDepth();
}


QAbstractItemModel * TocModels::model(QString const & type)
{
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


QModelIndex TocModels::currentIndex(QString const & type,
                                    DocIterator const & dit) const
{
	const_iterator it = models_.find(type);
	if (it == models_.end())
		return QModelIndex();
	return it.value()->modelIndex(dit);
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
	clear();
	if (!bv) {
		iterator end = models_.end();
		for (iterator it = models_.begin(); it != end;  ++it)
			it.value()->reset();
		names_->reset();
		return;
	}

	names_->blockSignals(true);
	names_->beginResetModel();
	names_->insertColumns(0, 1);
	// In the outliner, add Tocs from the master document
	TocBackend const & backend = bv->buffer().masterBuffer()->tocBackend();
	for (pair<const string, shared_ptr<Toc>> const & toc : backend.tocs()) {
		QString const type = toqstr(toc.first);

		// First, fill in the toc models.
		iterator mod_it = models_.find(type);
		if (mod_it == models_.end())
			mod_it = models_.insert(type, new TocModel(this));
		mod_it.value()->reset(toc.second);

		// Fill in the names_ model.
		QString const gui_name = toqstr(backend.outlinerName(toc.first));
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
