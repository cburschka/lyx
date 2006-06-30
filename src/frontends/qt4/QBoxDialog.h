// -*- C++ -*-
/**
 * \file QBoxDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBOXDIALOG_H
#define QBOXDIALOG_H

#include "ui/QBoxUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QBox;

class QBoxDialog : public QDialog, public Ui::QBoxUi {
	Q_OBJECT
public:
	QBoxDialog(QBox * form);
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void innerBoxChanged(const QString &);
	virtual void typeChanged(int);
	virtual void restoreClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QBox * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QBOXDIALOG_H
