/**
 * \file QBibtexDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QBIBTEXDIALOG_H
#define QBIBTEXDIALOG_H
 
#include <config.h>
 
#include "ui/QBibtexDialogBase.h"

class QBibtex;

class QBibtexDialog : public QBibtexDialogBase
{ Q_OBJECT

public:
	QBibtexDialog(QBibtex * form);

protected slots:
	virtual void change_adaptor();
	virtual void browsePressed();
	virtual void addPressed();
	virtual void deletePressed();
	virtual void styleChanged(const QString &);
	virtual void databaseChanged(); 

 
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QBibtex * form_;
};

#endif // QBIBTEXDIALOG_H
