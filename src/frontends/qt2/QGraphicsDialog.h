/**
 * \file QGraphicsDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QGRAPHICSDIALOG_H
#define QGRAPHICSDIALOG_H
 
#include <config.h>
 
#include "ui/QGraphicsDialogBase.h"

class QGraphics;

class QGraphicsDialog : public QGraphicsDialogBase
{ Q_OBJECT

public:
	QGraphicsDialog(QGraphics * form);

protected slots:
	virtual void change_adaptor();
	virtual void browseClicked();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QGraphics * form_;
};

#endif // QGRAPHICSDIALOG_H
