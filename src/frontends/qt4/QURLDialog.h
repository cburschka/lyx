// -*- C++ -*-
/**
 * \file QURLDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QURLDIALOG_H
#define QURLDIALOG_H

#include "ui_URLUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class UrlView;

class QURLDialog : public QDialog, public Ui::QURLUi {
	Q_OBJECT
public:
	QURLDialog(UrlView * form);
	~QURLDialog();

	virtual void show();
public Q_SLOTS:
	void changed_adaptor();
protected:
	void closeEvent(QCloseEvent *);
private:
	UrlView * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QURLDIALOG_H
