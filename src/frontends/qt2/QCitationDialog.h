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


#include "ui/QCitationDialogBase.h"
#include "controllers/biblio.h"

class QCitation;

class QCitationDialog : public QCitationDialogBase {
	Q_OBJECT

public:
	QCitationDialog(QCitation * form);

	~QCitationDialog();

	void setButtons();

protected slots:

	virtual void availableChanged();
	virtual void selectedChanged();
	virtual void next();
	virtual void previous();
	virtual void up();
	virtual void down();
	virtual void del();
	virtual void add();
	virtual void changed_adaptor();

private:
	void doFind(biblio::Direction dir);

private:
	QCitation * form_;
};

#endif // QCITATIOINDIALOG_H
