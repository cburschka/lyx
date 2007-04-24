// -*- C++ -*-
/**
 * \file QAboutDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QABOUTDIALOG_H
#define QABOUTDIALOG_H

#include "ui/AboutUi.h"
#include <QDialog>

namespace lyx {
namespace frontend {

class QAboutDialog : public QDialog, public Ui::QAboutUi {
	Q_OBJECT
public:
	QAboutDialog(QWidget * parent = 0, const char * name = 0,
		     bool modal = FALSE, Qt::WFlags fl = 0);
	~QAboutDialog();
};

} // namespace frontend
} // namespace lyx

#endif // QABOUTDIALOG_H
