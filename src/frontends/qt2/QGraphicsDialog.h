// -*- C++ -*-
/**
 * \file QGraphicsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QGRAPHICSDIALOG_H
#define QGRAPHICSDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QGraphicsDialogBase.h"

class QGraphics;

class QGraphicsDialog : public QGraphicsDialogBase
{ Q_OBJECT

public:
	QGraphicsDialog(QGraphics * form);

	virtual void show();

protected slots:
	virtual void change_adaptor();
	virtual void browse_clicked();
	virtual void get_clicked();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QGraphics * form_;
};

#endif // QGRAPHICSDIALOG_H
