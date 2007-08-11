// -*- C++ -*-
/**
 * \file QBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBIBITEM_CONTROLLER_H
#define QBIBITEM_CONTROLLER_H

#include "QDialogView.h"

#include "ui_BibitemUi.h"

#include <QDialog>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

class QBibitem;

class QBibitemDialog : public QDialog, public Ui::QBibitemUi {
	Q_OBJECT
public:

	QBibitemDialog(QBibitem * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBibitem * form_;
};


class ControlCommand;

class QBibitem
	: public QController<ControlCommand, QView<QBibitemDialog> >
{
public:
	friend class QBibitemDialog;

	QBibitem(Dialog &);

protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QBIBITEM_H
