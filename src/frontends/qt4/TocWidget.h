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

class QString;

namespace lyx {
namespace frontend {

class TocModels;

class TocWidget : public QWidget, public Ui::TocUi
{
	Q_OBJECT
public:
	TocWidget(TocModels & models, QWidget * parent = 0);

	/// Initialise GUI.
	void init(QString const & str);

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void updateView();

protected Q_SLOTS:
	///
	void setTocModel(size_t type);
	///
	void select(QModelIndex const & index);
	///
	void goTo(QModelIndex const &);

	void on_tocTV_activated(QModelIndex const &);
	void on_tocTV_clicked(QModelIndex const &);
	void on_updateTB_clicked();
	void on_depthSL_valueChanged(int depth);
	void on_typeCO_currentIndexChanged(int value);
	void on_moveUpTB_clicked();
	void on_moveDownTB_clicked();
	void on_moveInTB_clicked();
	void on_moveOutTB_clicked();

private:
	///
	void enableControls(bool enable = true);
	///
	int getIndexDepth(QModelIndex const & index, int depth = -1);
	///
	void setTreeDepth(int depth);

	/// depth of list shown
	int depth_;
	///
	TocModels & models_;
};

} // namespace frontend
} // namespace lyx

#endif // TOC_WIDGET_H
