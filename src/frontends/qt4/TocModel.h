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

#include "qt_helpers.h"

#include <QHash>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringList>

namespace lyx {

class Buffer;
class BufferView;
class DocIterator;
class Toc;
class TocItem;

namespace frontend {

class TocTypeModel : public QStandardItemModel
{
public:
	///
	TocTypeModel(QObject * parent = 0);
	///
	void reset();
};


class TocModel
{
public:
	///
	TocModel(QObject * parent = 0);
	///
	void reset(Toc const & toc);
	///
	void reset();
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
	TocTypeModel * model_;
	///
	QSortFilterProxyModel * sorted_model_;
	///
	bool is_sorted_;
	///
	QList<QModelIndex> toc_indexes_;
	///
	Toc const * toc_;
	///
	int maxdepth_;
	int mindepth_;
};


class TocModels: public QObject
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
	void updateBackend() const;
	///
	void sort(QString const & type, bool sort_it);
	///
	bool isSorted(QString const & type) const;

Q_SIGNALS:
	/// Signal that the internal toc_models_ has been reset.
	void modelReset();

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
	QSortFilterProxyModel * names_sorted_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
