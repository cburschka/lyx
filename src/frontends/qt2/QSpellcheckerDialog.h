/**
 * \file QSpellcheckerDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QSPELLCHECKERDIALOG_H
#define QSPELLCHECKERDIALOG_H

#include <config.h>

#include "ui/QSpellcheckerDialogBase.h"

#include "QSpellchecker.h"

class QSpellcheckerDialog : public QSpellcheckerDialogBase
{ Q_OBJECT

public:
	QSpellcheckerDialog(QSpellchecker * form);

protected slots:
	virtual void stop() {
		form_->stop();
	}

	virtual void acceptClicked() {
		form_->accept();
	}
	virtual void spellcheckClicked() {
		form_->spellcheck();
	}
	virtual void addClicked() {
		form_->add();
	}
	virtual void replaceClicked() {
		form_->replace();
	}
	virtual void ignoreClicked() {
		form_->ignore();
	}
	virtual void optionsClicked() {
		form_->options();
	}
	virtual void suggestionChanged(const QString &);
	virtual void replaceChanged(const QString &);

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QSpellchecker * form_;
};

#endif // QSPELLCHECKERDIALOG_H
