// -*- C++ -*-
/**
 * \file QVSpaceDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QVSPACEDIALOG_H
#define QVSPACEDIALOG_H

#include "ui/QVSpaceUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class QVSpace;

class QVSpaceDialog : public QDialog, public Ui::QVSpaceUi {
	Q_OBJECT

public:
	QVSpaceDialog(QVSpace * form);

public slots:
	void change_adaptor();

protected slots:
	void closeEvent(QCloseEvent *);
	void enableCustom(int);

private:
	QVSpace * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QVSPACEDIALOG_H
