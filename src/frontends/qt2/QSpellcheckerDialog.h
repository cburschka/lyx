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

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QSpellcheckerDialogBase.h"

class QSpellchecker;

class QSpellcheckerDialog : public QSpellcheckerDialogBase {
	Q_OBJECT
public:
	QSpellcheckerDialog(QSpellchecker * form);
protected slots:
	virtual void stop();
	virtual void acceptClicked();
	virtual void spellcheckClicked();
	virtual void addClicked();
	virtual void replaceClicked();
	virtual void ignoreClicked();
	virtual void suggestionChanged(const QString &);
	virtual void replaceChanged(const QString &);
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QSpellchecker * form_;
};

#endif // QSPELLCHECKERDIALOG_H
