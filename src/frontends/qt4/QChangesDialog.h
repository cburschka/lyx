// -*- C++ -*-
/**
 * \file QChangesDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCHANGESDIALOG_H
#define QCHANGESDIALOG_H

#include "ui/QChangesUi.h"
#include "QChanges.h"

#include <QCloseEvent>
#include <QDialog>

class QChanges;

namespace lyx {
namespace frontend {

class QChangesDialog : public QDialog, public Ui::QChangesUi {
	Q_OBJECT
public:

	QChangesDialog(QChanges * form);

protected slots:

	virtual void nextPressed();
	virtual void acceptPressed();
	virtual void rejectPressed();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QChanges * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QCHANGESDIALOG_H
