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
namespace frontend {

class TocModel : public QStandardItemModel
{
	Q_OBJECT

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

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
