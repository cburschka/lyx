// -*- C++ -*-
/**
 * \file QBibtexDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QBIBTEXDIALOG_H
#define QBIBTEXDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QBibtexDialogBase.h"

class QBibtex;

class QBibtexDialog : public QBibtexDialogBase
{
	Q_OBJECT

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
