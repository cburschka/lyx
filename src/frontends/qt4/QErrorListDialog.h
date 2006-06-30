// -*- C++ -*-
/**
 * \file QErrorListDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QERRORLISTDIALOG_H
#define QERRORLISTDIALOG_H

#include "ui/QErrorListUi.h"

#include <QDialog>
#include <QCloseEvent>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class QErrorList;

class QErrorListDialog : public QDialog, public Ui::QErrorListUi {
	Q_OBJECT
public:
	QErrorListDialog(QErrorList * form);
	~QErrorListDialog();
public Q_SLOTS:
	void select_adaptor(QListWidgetItem *);
protected:
	void closeEvent(QCloseEvent * e);
private:
	QErrorList * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOCERRORLIST_H
