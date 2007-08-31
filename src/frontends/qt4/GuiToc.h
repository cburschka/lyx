// -*- C++ -*-
/**
 * \file GuiToc.h
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

class GuiToc : public QObject, public ControlToc
{
	Q_OBJECT
public:

	GuiToc(Dialog &, QObject * parent = 0);
	virtual ~GuiToc() {}

	/// \c ControlToc inherited method.
	virtual bool initialiseParams(std::string const & data);
	///
	void update();
	///
	bool canOutline(int type) const;

	QStandardItemModel * tocModel(int type);
	///
	QModelIndex const getCurrentIndex(int type) const;
	///
	void goTo(int type, QModelIndex const & index);
	///
	int getType();
	///
	int getTocDepth(int type);

Q_SIGNALS:
	/// Signal that the internal toc_models_ has been reset.
	void modelReset();

private:
	///
	std::vector<TocModel *> toc_models_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOC_H
