// -*- C++ -*-
/**
 * \file QSpellcheckerDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSPELLCHECKERDIALOG_H
#define QSPELLCHECKERDIALOG_H

#include "ui/QSpellcheckerDialogBase.h"

class QSpellchecker;

class QSpellcheckerDialog : public QSpellcheckerDialogBase {
	Q_OBJECT
public:
	QSpellcheckerDialog(QSpellchecker * form);
public slots:
	virtual void suggestionChanged(const QString &);
 
protected slots:
	virtual void acceptClicked();
	virtual void addClicked();
	virtual void replaceClicked();
	virtual void ignoreClicked();
	virtual void replaceChanged(const QString &);
	virtual void reject();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QSpellchecker * form_;
};

#endif // QSPELLCHECKERDIALOG_H
