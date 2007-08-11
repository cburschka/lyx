// -*- C++ -*-
/**
 * \file QSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSPELLCHECKER_H
#define QSPELLCHECKER_H

#include "QDialogView.h"
#include "ui_SpellcheckerUi.h"

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


class ControlSpellchecker;

class QSpellchecker
	: public QController<ControlSpellchecker, QView<QSpellcheckerDialog> >
{
public:
	friend class QSpellcheckerDialog;

	QSpellchecker(Dialog &);

	/// update from controller
	void partialUpdate(int id);
private:
	void accept();
	void add();
	void ignore();
	void replace();

	/// Apply changes
	virtual void apply() {}
	///
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QSPELLCHECKER_H
