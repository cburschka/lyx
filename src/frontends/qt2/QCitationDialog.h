// -*- C++ -*-
/**
 * \file QCitationDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QCitationDialogBase.h"
#include "controllers/biblio.h"

class QCitation;

class QCitationDialog : public QCitationDialogBase
{ Q_OBJECT

public:
	QCitationDialog(QCitation * form);
	~QCitationDialog();

protected slots:
	virtual void slotBibHighlighted(int sel);
	virtual void slotBibSelected(int sel);
	virtual void slotCiteHighlighted(int sel);
	virtual void slotAddClicked();
	virtual void slotDelClicked();
	virtual void slotUpClicked();
	virtual void slotDownClicked();
	virtual void slotPreviousClicked();
	virtual void slotNextClicked();
	virtual void changed_adaptor();

private:
	void doFind(biblio::Direction dir);

private:
	QCitation * form_;
};

#endif // QCITATIOINDIALOG_H
