/**
 * \file QAboutDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QABOUTDIALOG_H
#define QABOUTDIALOG_H
#include "ui/QAboutDialogBase.h"

class QAboutDialog : public QAboutDialogBase
{
Q_OBJECT
public:
	QAboutDialog(QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QAboutDialog();
};

#endif // QABOUTDIALOG_H
