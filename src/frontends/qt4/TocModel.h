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

#include <QList>
#include <QStandardItemModel>
#include <QStringList>

namespace lyx {

class Buffer;
class BufferView;
class DocIterator;
class Toc;
class TocItem;

namespace frontend {

class TocModel : public QStandardItemModel
{
public:
	///
	TocModel(Toc const & toc);
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
	QList<QModelIndex> toc_indexes_;
	///
	Toc const & toc_;
	///
	int maxdepth_;
	int mindepth_;
};


class TocModels: public QObject
{
	Q_OBJECT
public:
	///
	TocModels(): bv_(0) {}
	///
	~TocModels() { clear(); }
	///
	void reset(BufferView const * bv);
	///
	int depth(int type);
	///
	QStandardItemModel * model(int type);
	///
	QModelIndex currentIndex(int type) const;
	///
	void goTo(int type, QModelIndex const & index) const;
	///
	void init(Buffer const & buffer);
	/// Test if outlining operation is possible
	bool canOutline(int type) const;
	/// Return the list of types available
	QStringList const & typeNames() const { return type_names_; }
	///
	void updateBackend() const;
	///
	int decodeType(QString const & str) const;

Q_SIGNALS:
	/// Signal that the internal toc_models_ has been reset.
	void modelReset();

private:
	///
	void clear();
	///
	BufferView const * bv_;
	///
	QList<TocModel *> models_;
	///
	QStringList types_;
	///
	QStringList type_names_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
