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


#include "ui/QVSpaceDialogBase.h"

class QVSpace;

class QVSpaceDialog : public QVSpaceDialogBase {
	Q_OBJECT

public:
	QVSpaceDialog(QVSpace * form);

public slots:
	void change_adaptor();

protected:
	void closeEvent(QCloseEvent *);
	void enableCustom(int);

private:
	QVSpace * form_;
};

#endif // QVSPACEDIALOG_H
