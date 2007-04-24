// -*- C++ -*-
/**
 * \file QSpellcheckerDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSPELLCHECKERDIALOG_H
#define QSPELLCHECKERDIALOG_H

#include "ui/SpellcheckerUi.h"

#include <QDialog>
#include <QCloseEvent>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class QSpellchecker;

class QSpellcheckerDialog: public QDialog, public Ui::QSpellcheckerUi {
	Q_OBJECT
public:
	QSpellcheckerDialog(QSpellchecker * form);
public Q_SLOTS:
	virtual void suggestionChanged(QListWidgetItem *);

protected Q_SLOTS:
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

} // namespace frontend
} // namespace lyx

#endif // QSPELLCHECKERDIALOG_H
