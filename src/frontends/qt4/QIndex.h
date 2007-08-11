// -*- C++ -*-
/**
 * \file QIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QINDEX_H
#define QINDEX_H

#include "QDialogView.h"
#include "ui_IndexUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QIndex;

class QIndexDialog : public QDialog, public Ui::QIndexUi {
	Q_OBJECT
public:
	QIndexDialog(QIndex * form);

protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void reject();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QIndex * form_;
};


class ControlCommand;

class QIndex :
	public QController<ControlCommand, QView<QIndexDialog> >
{
public:
	friend class QIndexDialog;

	QIndex(Dialog &, docstring const & title, QString const & label);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	///
	QString const label_;
};

} // namespace frontend
} // namespace lyx

#endif // QINDEX_H
