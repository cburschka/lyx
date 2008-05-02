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

#include "TocBackend.h"

#include "qt_helpers.h"

#include <QStandardItemModel>

#include <map>

namespace lyx {

class BufferView;

namespace frontend {

class TocModel : public QStandardItemModel
{
public:
	///
	TocModel() {}
	///
	TocModel(Toc const & toc) { populate(toc); }
	///
	void clear();
	///
	void populate(Toc const & toc);
	///
	TocIterator tocIterator(QModelIndex const & index) const;
	///
	QModelIndex modelIndex(TocIterator const & it) const;
	///
	int modelDepth() const;

private:
	///
	void populate(TocIterator & it, TocIterator const & end,
		QModelIndex const & parent);
	///
	typedef std::map<QModelIndex, TocIterator> TocMap;
	///
	typedef std::map<TocIterator, QModelIndex> ModelMap;
	///
	TocMap toc_map_;
	///
	ModelMap model_map_;
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
	/// Return the guiname from a given cmdName of the TOC param
	QString guiName(std::string const & type) const;
	///
	BufferView const * bv_;
	///
	std::vector<TocModel *> models_;
	///
	QStringList types_;
	///
	QStringList type_names_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
