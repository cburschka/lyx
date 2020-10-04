// -*- C++ -*-
/**
 * \file GuiBranches.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIBRANCHES_H
#define GUIBRANCHES_H

#include "BranchList.h"
#include "ButtonController.h"

#include "ui_BranchesUnknownUi.h"
#include "ui_BranchesUi.h"

#include <QDialog>
#include <QWidget>

class QTreeWidgetItem;

namespace lyx {

class BufferParams;

namespace frontend {

class BranchesUnknownDialog : public QDialog, public Ui::BranchesUnknownUi
{
public:
	BranchesUnknownDialog(QWidget * parent) : QDialog(parent)
	{
		Ui::BranchesUnknownUi::setupUi(this);
		QDialog::setModal(true);
	}
};

class GuiBranches : public QWidget, public Ui::BranchesUi
{
	Q_OBJECT
public:
	GuiBranches(QWidget * parent = 0);

	void update(BufferParams const & params);
	void apply(BufferParams & params) const;
	void setUnknownBranches(QStringList const & b) { unknown_branches_ = b; }

	bool eventFilter(QObject * obj, QEvent * event) override;

Q_SIGNALS:
	void changed();
	void renameBranches(docstring const &, docstring const &);
	void okPressed();

protected:
	void toggleBranch(QTreeWidgetItem *);
	void toggleColor(QTreeWidgetItem *);
	void toggleSuffix(QTreeWidgetItem *);
	void updateView();

protected Q_SLOTS:
	void on_newBranchLE_textChanged(QString);
	void on_addBranchPB_pressed();
	void on_removePB_pressed();
	void on_renamePB_pressed();
	void on_activatePB_pressed();
	void on_branchesTW_itemDoubleClicked(QTreeWidgetItem *, int);
	void on_branchesTW_itemSelectionChanged();
	void on_colorPB_clicked();
	void on_suffixPB_pressed();
	void on_unknownPB_pressed();
	void addUnknown();
	void addAllUnknown();
	void unknownBranchSelChanged();

private:
	/// Contains all legal branches for this doc
	BranchList branchlist_;
	///
	BranchesUnknownDialog * undef_;
	///
	ButtonController undef_bc_;
	///
	QStringList unknown_branches_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBRANCHES_H
