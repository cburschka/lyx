// -*- C++ -*-
/**
 * \file QTexinfoDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTEXINFODIALOG_H
#define QTEXINFODIALOG_H

#include "ui/QTexinfoUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QTexinfo;

class QTexinfoDialog : public QDialog, public Ui::QTexinfoUi {
	Q_OBJECT
public:
	QTexinfoDialog(QTexinfo * form);
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void rescanClicked();
	virtual void viewClicked();
	virtual void update();
	virtual void enableViewPB();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QTexinfo * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QTEXINFODIALOG_H
