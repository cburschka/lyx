// -*- C++ -*-
/**
 * \file QDocumentDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBRANCHES_H
#define QBRANCHES_H

#include "ui/BranchesUi.h"
#include "QDocument.h"
#include "BranchList.h"

#include <QWidget>

#include <vector>
#include <string>


class Q3ListViewItem;

class BufferParams;

namespace lyx {
namespace frontend {


class QBranches: public QWidget, public Ui::BranchesUi
{
	Q_OBJECT
public:
	QBranches(QWidget * parent=0, Qt::WFlags f=0);
	~QBranches();

	void update(BufferParams const & params);
	void apply(BufferParams & params) const;

signals:
	void changed();

protected:
	void toggleBranch(Q3ListViewItem * selItem);
	void update();

protected slots:
	void on_addBranchPB_pressed();
	void on_removePB_pressed();
	void on_activatePB_pressed();
	void on_branchesLV_doubleClicked(Q3ListViewItem *);
	void on_colorPB_clicked();

private:

	/// Contains all legal branches for this doc
	BranchList branchlist_;

};

} // namespace frontend
} // namespace lyx

#endif // DOCUMENTDIALOG_H
