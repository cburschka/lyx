// -*- C++ -*-
/**
 * \file GuiThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHESAURUS_H
#define QTHESAURUS_H

#include "GuiDialogView.h"
#include "ui_ThesaurusUi.h"

#include <QDialog>
#include <QCloseEvent>


class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class ControlThesaurus;

class GuiThesaurus;

class GuiThesaurusDialog : public QDialog, public Ui::ThesaurusUi {
	Q_OBJECT
public:
	GuiThesaurusDialog(GuiThesaurus * form);

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
	GuiThesaurus * form_;
};


///
class GuiThesaurus
	: public QController<ControlThesaurus, GuiView<GuiThesaurusDialog> >
{
public:
	///
	friend class GuiThesaurusDialog;
	///
	GuiThesaurus(Dialog &);
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
