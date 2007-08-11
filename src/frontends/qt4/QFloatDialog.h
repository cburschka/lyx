// -*- C++ -*-
/**
 * \file QFloatDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QFLOATDIALOG_H
#define QFLOATDIALOG_H

#include "ui_FloatUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QFloat;

class QFloatDialog : public QDialog, public Ui::QFloatUi {
	Q_OBJECT
public:
	QFloatDialog(QFloat * form);

protected Q_SLOTS:
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QFloat * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QFLOATDIALOG_H
