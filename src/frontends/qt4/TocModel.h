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
#include <string>

namespace lyx {
namespace frontend {

typedef TocBackend::Toc::const_iterator TocIterator;

class TocModel: public QStandardItemModel {
	Q_OBJECT

public:
	///
	TocModel() {}
	///
	TocModel(TocBackend::Toc const & toc);
	///
	~TocModel() {}
	///
	TocModel const & operator=(TocBackend::Toc const & toc);
	///
	void clear();
	///
	void populate(TocBackend::Toc const & toc);
	///
	TocIterator const tocIterator(QModelIndex const & index) const;
	///
	QModelIndex const modelIndex(TocIterator const & it) const;

private:
	///
	void populate(TocIterator & it,
		TocIterator const & end,
		QModelIndex const & parent);
	///
	typedef std::map<QModelIndex, TocIterator> TocMap;
	///
	typedef std::map<TocIterator, QModelIndex> ModelMap;
	///
	TocMap toc_map_;
	///
	ModelMap model_map_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
