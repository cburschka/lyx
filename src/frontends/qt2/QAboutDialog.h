// -*- C++ -*-
/**
 * \file QAboutDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QABOUTDIALOG_H
#define QABOUTDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QAboutDialogBase.h"

class QAboutDialog : public QAboutDialogBase {
	Q_OBJECT
public:
	QAboutDialog(QWidget * parent = 0, const char * name = 0,
		     bool modal = FALSE, WFlags fl = 0);
	~QAboutDialog();
};

#endif // QABOUTDIALOG_H
