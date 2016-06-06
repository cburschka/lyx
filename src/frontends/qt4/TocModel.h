// -*- C++ -*-
/**
 * \file TocModel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOCMODEL_H
#define TOCMODEL_H

#include "Toc.h"

#include <QHash>
#include <QSortFilterProxyModel>

namespace lyx {

class Buffer;
class BufferView;
class DocIterator;

namespace frontend {

class TocTypeModel;

/// A class that adapt the TocBackend of a Buffer into standard Qt models for
/// GUI visualisation.
/// There is one TocModel per list in the TocBackend.
class TocModel
{
	/// noncopyable
	TocModel(TocModel const &);
	void operator=(TocModel const &);
public:
	///
	TocModel(QObject * parent);
	///
	void reset(std::shared_ptr<Toc const>);
	///
	void reset();
	///
	void updateItem(DocIterator const & dit);
	///
	void clear();
	///
	QAbstractItemModel * model();
	///
	QAbstractItemModel const * model() const;
	///
	void sort(bool sort_it);
	///
	bool isSorted() const { return is_sorted_; }
	///
	TocItem const & tocItem(QModelIndex const & index) const;
	///
	QModelIndex modelIndex(DocIterator const & dit) const;
	///
	int modelDepth() const;

private:
	///
	void populate(unsigned int & index, QModelIndex const & parent);
	///
	void setString(TocItem const & item, QModelIndex index);
	///
	TocTypeModel * model_;
	///
	QSortFilterProxyModel * sorted_model_;
	///
	bool is_sorted_;
	///
	std::shared_ptr<Toc const> toc_;
	///
	int maxdepth_;
	///
	int mindepth_;
};


/// A filter to sort the models alphabetically but with
/// the table of contents on top.
class TocModelSortProxyModel : public QSortFilterProxyModel
{
public:
	TocModelSortProxyModel(QObject * w) 
		: QSortFilterProxyModel(w)
	{}

	bool lessThan (const QModelIndex & left, const QModelIndex & right) const
	{
		if (left.model()->data(left, Qt::UserRole).toString()
			  == QString("tableofcontents"))
			return true;
		else if (right.model()->data(right, Qt::UserRole).toString()
			  == QString("tableofcontents"))
			return false;
		else
			return QSortFilterProxyModel::lessThan(left, right);
	}
};



/// A container for the different TocModels.
class TocModels : public QObject
{
	Q_OBJECT
public:
	///
	TocModels();
	///
	void reset(BufferView const * bv);
	///
	int depth(QString const & type);
	///
	QAbstractItemModel * model(QString const & type);
	///
	QAbstractItemModel * nameModel();
	///
	QModelIndex currentIndex(QString const & type) const;
	///
	void goTo(QString const & type, QModelIndex const & index) const;
	///
	void init(Buffer const & buffer);
	///
	void updateItem(QString const & type, DocIterator const & dit);
	///
	void sort(QString const & type, bool sort_it);
	///
	bool isSorted(QString const & type) const;
	/// the item that is currently selected
	TocItem const currentItem(QString const & type,
		QModelIndex const & index) const;

private:
	typedef QHash<QString, TocModel *>::const_iterator const_iterator;
	typedef QHash<QString, TocModel *>::iterator iterator;
	///
	void clear();
	///
	BufferView const * bv_;
	///
	QHash<QString, TocModel *> models_;
	///
	TocTypeModel * names_;
	///
	TocModelSortProxyModel * names_sorted_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
