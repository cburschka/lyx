// -*- C++ -*-
/**
 * \file QErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QERRORLIST_H
#define QERRORLIST_H

#include "QDialogView.h"
#include "ui/ErrorListUi.h"

#include <QDialog>

class QListWidgetItem;
class QCloseEvent;
class QShowEvent;

namespace lyx {
namespace frontend {

class QErrorList;

class QErrorListDialog : public QDialog, public Ui::QErrorListUi {
	Q_OBJECT
public:
	QErrorListDialog(QErrorList * form);

public Q_SLOTS:
	void select_adaptor();
protected:
	void closeEvent(QCloseEvent *);
	void showEvent(QShowEvent *);
private:
	QErrorList * form_;
};


class ControlErrorList;

class QErrorList :
	public QController<ControlErrorList, QView<QErrorListDialog> >
{
public:
	friend class QErrorListDialog;

	QErrorList(Dialog &);
private:
	/// select an entry
	void select(QListWidgetItem *);
	/// required apply
	virtual void apply() {}
	/// build dialog
	virtual void build_dialog();
	/// update contents
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QERRORLIST_H
