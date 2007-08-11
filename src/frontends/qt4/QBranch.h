// -*- C++ -*-
/**
 * \file QBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBRANCH_H
#define QBRANCH_H

#include "QDialogView.h"
#include "ui_BranchUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QBranch;

class QBranchDialog : public QDialog, public Ui::QBranchUi {
	Q_OBJECT
public:
	QBranchDialog(QBranch * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBranch * form_;
};

class ControlBranch;

/** This class provides a QT implementation of the Branch Dialog.
 */
class QBranch : public QController<ControlBranch, QView<QBranchDialog> >
{
public:
	friend class QBranchDialog;

	/// Constructor
	QBranch(Dialog &);
private:
	///  Apply changes
	virtual void apply();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QBRANCH_H
