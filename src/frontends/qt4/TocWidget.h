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

#include "ui_TocUi.h"

#include "Cursor.h"

#include <QWidget>

class QModelIndex;
class QString;

namespace lyx {
namespace frontend {

class GuiView;

class TocWidget : public QWidget, public Ui::TocUi
{
	Q_OBJECT
public:
	///
	TocWidget(GuiView & gui_view, QWidget * parent = 0);

	/// Initialise GUI.
	void init(QString const & str);
	///
	void doDispatch(Cursor & cur, FuncRequest const & fr);
	///
	bool getStatus(Cursor & cur, FuncRequest const & fr, FuncStatus & status)
		const;

public Q_SLOTS:
	/// Update the display of the dialog whilst it is still visible.
	void updateView();

protected Q_SLOTS:
	///
	void select(QModelIndex const & index);
	///
	void goTo(QModelIndex const &);

	void on_tocTV_activated(QModelIndex const &);
	void on_tocTV_pressed(QModelIndex const &);
	void on_updateTB_clicked();
	void on_sortCB_stateChanged(int state);
	void on_persistentCB_stateChanged(int state);
	void on_depthSL_valueChanged(int depth);
	void on_typeCO_currentIndexChanged(int value);
	void on_moveUpTB_clicked();
	void on_moveDownTB_clicked();
	void on_moveInTB_clicked();
	void on_moveOutTB_clicked();

	void showContextMenu(const QPoint & pos);

private:
	///
	void enableControls(bool enable = true);
	///
	void setTreeDepth(int depth);
	///
	void outline(int func_code);
	/// finds the inset that is connected to the current item,
	/// if any, otherwise return null
	Inset * itemInset() const;
	///
	QString current_type_;

	/// depth of list shown
	int depth_;
	/// persistence of uncollapsed nodes in toc view
	bool persistent_;
	///
	GuiView & gui_view_;
};

} // namespace frontend
} // namespace lyx

#endif // TOC_WIDGET_H
