// -*- C++ -*-
/**
 * \file QIndexDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QINDEXDIALOG_H
#define QINDEXDIALOG_H

#include "ui/QIndexUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QIndex;

class QIndexDialog : public QDialog, public Ui::QIndexUi {
	Q_OBJECT
public:
	QIndexDialog(QIndex * form);

	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void reject();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QIndex * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QINDEXDIALOG_H
