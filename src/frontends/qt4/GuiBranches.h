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

#include "GuiDocument.h"
#include "ui_BranchesUi.h"
#include "BranchList.h"

#include <QWidget>

class QTreeWidgetItem;

namespace lyx {

class BufferParams;

namespace frontend {

class GuiBranches : public QWidget, public Ui::BranchesUi
{
	Q_OBJECT
public:
	GuiBranches(QWidget * parent = 0);

	void update(BufferParams const & params);
	void apply(BufferParams & params) const;

Q_SIGNALS:
	void changed();

protected:
	void toggleBranch(QTreeWidgetItem *);
	void toggleColor(QTreeWidgetItem *);
	void updateView();

protected Q_SLOTS:
	void on_addBranchPB_pressed();
	void on_removePB_pressed();
	void on_activatePB_pressed();
	void on_branchesTW_itemDoubleClicked(QTreeWidgetItem *, int);
	void on_colorPB_clicked();

private:
	/// Contains all legal branches for this doc
	BranchList branchlist_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIBRANCHES_H
