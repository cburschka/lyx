// -*- C++ -*-
/**
 * \file QToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTOC_H
#define QTOC_H

#include "ControlToc.h"

#include <QStandardItemModel>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class ControlToc;
class TocModel;

class QToc : public ControlToc
{
public:

	QToc(Dialog &);

	void update();

	void updateToc(int type);

	bool canOutline();
	
	QStandardItemModel * tocModel();
	QStandardItemModel * setTocModel(int type);

	QStringListModel * typeModel()
	{ return &type_model_; }

	void goTo(QModelIndex const & index);

	void move(toc::OutlineOp const operation, QModelIndex & index);

private:

	std::vector<TocModel *> toc_models_;

	QStringListModel type_model_;

	int type_;
	int outline_type_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOC_H
