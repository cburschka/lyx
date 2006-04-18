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

#include "toc.h"

#include "qt_helpers.h"

#include <QStandardItemModel>

#include <map>
#include <string>

namespace lyx {
namespace frontend {

class TocModel: public QStandardItemModel {
	Q_OBJECT
public:
	///
	TocModel() {}
	///
	TocModel(toc::Toc const & toc_list);
	///
	~TocModel() {}
	///
	TocModel const & operator=(toc::Toc const & toc_list);
	///
	void clear();
	///
	void populate(toc::Toc const & toc_list);
	///
	toc::TocItem const item(QModelIndex const & index) const;
	///
	QModelIndex const index(std::string const & toc_str) const;

private:
	///
	void populate(toc::Toc::const_iterator & iter,
		toc::Toc::const_iterator const & end,
		QModelIndex const & parent);

	typedef std::map<QModelIndex, toc::TocItem> ItemMap;
	///
	typedef std::map<std::string, QModelIndex> IndexMap;
	///
	ItemMap item_map_;
	IndexMap index_map_;
};

} // namespace frontend
} // namespace lyx

#endif // TOCMODEL_H
