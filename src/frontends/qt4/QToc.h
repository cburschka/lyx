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

#include <QObject>
#include <QStandardItemModel>
#include <QStringListModel>

namespace lyx {
namespace frontend {

class ControlToc;
class TocModel;

class QToc : public QObject, public ControlToc
{
	Q_OBJECT
public:

	QToc(Dialog &);

	void update();
	///
	void updateToc();
	///
	void updateType();

	bool canOutline();
	
	QStandardItemModel * tocModel();
	QStandardItemModel * setTocModel(int type);

	QStringListModel * typeModel()
	{ return &type_model_; }

	///
	QModelIndex const getCurrentIndex();
	///
	void goTo(QModelIndex const & index);
	///
	int getType();
	///
	int getTocDepth();

Q_SIGNALS:
	/// Signal that the internal toc_models_ has been reset.
	void modelReset();

private:
	///
	std::vector<TocModel *> toc_models_;
	///
	QStringListModel type_model_;
	///
	int type_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOC_H
