// -*- C++ -*-
/**
 * \file QGraphicsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QGRAPHICSDIALOG_H
#define QGRAPHICSDIALOG_H


#include "ui/QGraphicsDialogBase.h"

class QGraphics;

class QGraphicsDialog : public QGraphicsDialogBase {
	Q_OBJECT
public:
	QGraphicsDialog(QGraphics * form);

	virtual void show();
protected slots:
	virtual void change_adaptor();
	virtual void change_bb();
	virtual void change_WUnit();
	virtual void browse_clicked();
	virtual void getBB_clicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QGraphics * form_;
};

#endif // QGRAPHICSDIALOG_H
