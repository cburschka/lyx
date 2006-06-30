// -*- C++ -*-
/**
 * \file QSendtoDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSENDTODIALOG_H
#define QSENDTODIALOG_H

#include "ui/QSendtoUi.h"

#include <QDialog>
#include <QCloseEvent>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class QSendto;

class QSendtoDialog : public QDialog, public Ui::QSendtoUi {
	Q_OBJECT
public:
	QSendtoDialog(QSendto * form);
protected Q_SLOTS:
	virtual void changed_adaptor();
	virtual void slotFormatHighlighted(QListWidgetItem *) {}
	virtual void slotFormatSelected(QListWidgetItem *) {}
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QSendto * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QSENDTODIALOG_H
