// -*- C++ -*-
/**
 * \file QTocDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTOCDIALOG_H
#define QTOCDIALOG_H

#include "ui/QTocUi.h"
#include "controllers/ControlToc.h"

#include <QDialog>
#include <QCloseEvent>

class QTreeWidget;
class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class QToc;

class QTocDialog : public QDialog, public Ui::QTocUi {
	Q_OBJECT
public:
	QTocDialog(QToc * form);
	~QTocDialog();

	/// update the listview
	void updateToc(bool newdepth=false);

	/// update the float types
	void updateType();

protected slots:
	///
	void on_tocTW_currentItemChanged(QTreeWidgetItem * current,
		QTreeWidgetItem * previous);

	void on_closePB_clicked();
	void on_updatePB_clicked();
	void on_depthSL_valueChanged(int depth);
	void on_typeCO_activated(int value);
	void on_moveUpPB_clicked();
	void on_moveDownPB_clicked();
	void on_moveInPB_clicked();
	void on_moveOutPB_clicked();

protected:
	void enableButtons(bool enable = true);
	void closeEvent(QCloseEvent * e);

private:

	void populateItem(QTreeWidgetItem * parentItem, toc::Toc::const_iterator& iter);

	QToc * form_;

	/// depth of list shown
	int depth_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOCDIALOG_H
