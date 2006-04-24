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

#include "ui/QBranchDialogBase.h"

namespace lyx {
namespace frontend {

class QBranch;

class QBranchDialog : public QBranchDialogBase {
	Q_OBJECT
public:
	QBranchDialog(QBranch * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBranch * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QBRANCHDIALOG_H
