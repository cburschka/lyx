// -*- C++ -*-
/**
 * \file QBranchDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBRANCHDIALOG_H
#define QBRANCHDIALOG_H

#include "ui/QBranchUi.h"

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

} // namespace frontend
} // namespace lyx

#endif // QBRANCHDIALOG_H
