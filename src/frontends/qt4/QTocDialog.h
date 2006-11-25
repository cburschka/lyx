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

class QTreeViewItem;

namespace lyx {
namespace frontend {

class QToc;

class QTocDialog : public QDialog, public Ui::QTocUi, public Dialog::View  {
	Q_OBJECT
public:
	QTocDialog(Dialog &, QToc * form);

	~QTocDialog();

	virtual void apply();

	/// Hide the dialog from sight
	void hide();

	/// Redraw the dialog (e.g. if the colors have been remapped).
	void redraw() {}

	/// Create the dialog if necessary, update it and display it.
	void show();

	/// Update the display of the dialog whilst it is still visible.
	void update();

	/// Update Gui of the display.
	void updateGui();

	/// \return true if the dialog is visible.
	bool isVisible() const;

protected Q_SLOTS:
	///
	void select(QModelIndex const & index);
	///
	void selectionChanged(const QModelIndex & current,
		const QModelIndex & previous);

	void on_closePB_clicked();
	void on_updatePB_clicked();
	void on_depthSL_valueChanged(int depth);
	void on_typeCO_activated(int value);
	void on_moveUpPB_clicked();
	void on_moveDownPB_clicked();
	void on_moveInPB_clicked();
	void on_moveOutPB_clicked();

protected:
	///
	void enableButtons(bool enable = true);
	/// Reconnects the selection model change signal when TOC changed.
	void reconnectSelectionModel();
	///
	int getIndexDepth(QModelIndex const & index, int depth = -1);
	///
	void setTreeDepth(int depth = -1);

private:

	QToc * form_;

	/// depth of list shown
	int depth_;
};

} // namespace frontend
} // namespace lyx

#endif // QTOCDIALOG_H
