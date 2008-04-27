// -*- C++ -*-
/**
 * \file TocWidget.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_WIDGET_H
#define TOC_WIDGET_H

#include "GuiDialog.h"
#include "GuiToc.h"
#include "ui_TocUi.h"


namespace lyx {
namespace frontend {

class GuiToc;

class TocWidget : public QWidget, public Ui::TocUi
{
	Q_OBJECT
public:
	TocWidget(GuiToc & form, QWidget * parent = 0);

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void updateView();

	/// Update Gui of the display.
	void updateGui(int selected_type);

protected Q_SLOTS:
	///
	void setTocModel(size_t type);
	///
	void select(QModelIndex const & index);
	///
	void selectionChanged(const QModelIndex & current,
		const QModelIndex & previous);

	void on_updateTB_clicked();
	void on_depthSL_valueChanged(int depth);
	void on_typeCO_currentIndexChanged(int value);
	void on_moveUpTB_clicked();
	void on_moveDownTB_clicked();
	void on_moveInTB_clicked();
	void on_moveOutTB_clicked();
	void setTreeDepth() { setTreeDepth(depth_); }

protected:
	///
	void enableControls(bool enable = true);
	///
	int getIndexDepth(QModelIndex const & index, int depth = -1);
	///
	void setTreeDepth(int depth);

private:
	/// Reconnects the selection model change signal when TOC changed.
	void reconnectSelectionModel();
	/// Disconnects the selection model.
	//This is a workaround for a problem of signals blocking.
	void disconnectSelectionModel();

	/// depth of list shown
	int depth_;
	///
	GuiToc & form_;
};

} // namespace frontend
} // namespace lyx

#endif // TOC_WIDGET_H
