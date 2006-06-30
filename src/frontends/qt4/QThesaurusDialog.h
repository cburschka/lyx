// -*- C++ -*-
/**
 * \file QThesaurusDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHESAURUSDIALOG_H
#define QTHESAURUSDIALOG_H

#include "ui/QThesaurusUi.h"

#include <QDialog>
#include <QCloseEvent>

class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class QThesaurus;

class QThesaurusDialog : public QDialog, public Ui::QThesaurusUi {
	Q_OBJECT
public:
	QThesaurusDialog(QThesaurus * form);

	void updateLists();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void entryChanged();
	virtual void replaceClicked();
	virtual void selectionChanged();
	virtual void selectionClicked(QTreeWidgetItem *, int);
	virtual void itemClicked(QTreeWidgetItem *, int);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QThesaurus * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QTHESAURUSDIALOG_H
