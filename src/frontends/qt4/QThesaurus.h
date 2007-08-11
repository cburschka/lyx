// -*- C++ -*-
/**
 * \file QThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHESAURUS_H
#define QTHESAURUS_H

#include "QDialogView.h"
#include "ui_ThesaurusUi.h"

#include <QDialog>
#include <QCloseEvent>


class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class ControlThesaurus;

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


///
class QThesaurus
	: public QController<ControlThesaurus, QView<QThesaurusDialog> >
{
public:
	///
	friend class QThesaurusDialog;
	///
	QThesaurus(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// replace the word
	void replace();
};

} // namespace frontend
} // namespace lyx

#endif // QTHESAURUS_H
