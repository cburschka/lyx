// -*- C++ -*-
/**
 * \file QThesaurusDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTHESAURUSDIALOG_H
#define QTHESAURUSDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QThesaurusDialogBase.h"

class QThesaurus;
class QListViewItem;

class QThesaurusDialog : public QThesaurusDialogBase {
	Q_OBJECT
public:
	QThesaurusDialog(QThesaurus * form);

	void updateLists();
protected slots:
	virtual void change_adaptor();
	virtual void entryChanged();
	virtual void replaceClicked();
	virtual void selectionChanged(QListViewItem *);
	virtual void selectionClicked(QListViewItem *);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QThesaurus * form_;
};

#endif // QTHESAURUSDIALOG_H
