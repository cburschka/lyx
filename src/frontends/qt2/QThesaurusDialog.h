/**
 * \file QThesaurusDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTHESAURUSDIALOG_H
#define QTHESAURUSDIALOG_H
 
#include <config.h>
 
#include "ui/QThesaurusDialogBase.h"

class QThesaurus;
class QListViewItem;

class QThesaurusDialog : public QThesaurusDialogBase
{ Q_OBJECT

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
