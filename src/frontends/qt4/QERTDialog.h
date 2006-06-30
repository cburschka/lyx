// -*- C++ -*-
/**
 * \file QERTDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QERTDIALOG_H
#define QERTDIALOG_H

#include "ui/QERTUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class QERT;

class QERTDialog : public QDialog, public Ui::QERTUi {
	Q_OBJECT
public:
	QERTDialog(QERT * form);
protected Q_SLOTS:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QERT * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QERTDIALOG_H
