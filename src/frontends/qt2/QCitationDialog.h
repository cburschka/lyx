/**
 * \file QCitationDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QCITATIONDIALOG_H
#define QCITATIONDIALOG_H

#include <config.h>
#include "support/lstrings.h"

#include "QCitation.h"
#include "ui/QCitationDialogBase.h"
#include "controllers/biblio.h"

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

	virtual void changed_adaptor() {
		form_->changed();
	}

private:
	void doFind(biblio::Direction dir);

private:
	QCitation * form_;
};

#endif // QCITATIOINDIALOG_H
