/**
 * \file QCitationDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

class QCitation;

#include <config.h>
#include "support/lstrings.h"
 
#include "ui/QCitationDialogBase.h"
#include "controllers/biblio.h"

class QCitationDialog : public QCitationDialogBase
{ Q_OBJECT

public:
	QCitationDialog(QCitation * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QCitationDialog();

protected slots:
	virtual void slotBibSelected(int sel);
	virtual void slotCiteSelected(int sel);
	virtual void slotAddClicked();
	virtual void slotDelClicked();
	virtual void slotUpClicked();
	virtual void slotDownClicked();
	virtual void slotPreviousClicked();
	virtual void slotNextClicked();
	virtual void slotCitationStyleSelected(int);
	virtual void slotTextBeforeReturn();
	virtual void slotTextAfterReturn();

private:
	void doFind(biblio::Direction dir);
    
private:
	QCitation * form_;
};

#endif // QCITATIOINDIALOG_H
