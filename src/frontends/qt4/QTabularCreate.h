// -*- C++ -*-
/**
 * \file QTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTABULARCREATE_H
#define QTABULARCREATE_H

#include "QDialogView.h"
#include "ui_TabularCreateUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class QTabularCreate;

class QTabularCreateDialog : public QDialog, public Ui::QTabularCreateUi {
	Q_OBJECT
public:
	QTabularCreateDialog(QTabularCreate * form);
protected Q_SLOTS:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);
private:
	QTabularCreate * form_;
};


class ControlTabularCreate;

class QTabularCreate
	: public QController<ControlTabularCreate, QView<QTabularCreateDialog> >
{
public:
	///
	friend class QTabularCreateDialog;
	///
	QTabularCreate(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QTABULARCREATE_H
