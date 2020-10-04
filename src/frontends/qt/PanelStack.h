// -*- C++ -*-
/**
 * \file PanelStack.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PANELSTACK_H
#define PANELSTACK_H

#include "FancyLineEdit.h"

#include <QHash>
#include <QWidget>

class QAbstractButton;
class QHideEvent;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

namespace lyx {
namespace frontend {


class PanelStack : public QWidget
{
	Q_OBJECT
public:
	///
	PanelStack(QWidget * parent = 0);

	/// add a category with no associated panel
	void addCategory(QString const & name, QString const & parent = QString());
	/// add a widget panel with a given name, under the given parent
	void addPanel(QWidget * panel, QString const & name,
		QString const & parent = QString());
	/// show or hide panel
	void showPanel(QString const & name, bool show);
	/// set current panel by logical name
	void setCurrentPanel(QString const &);
	///
	bool isCurrentPanel(QString const & name) const;
	///
	QSize sizeHint() const override;

public Q_SLOTS:
	/// the option filter changed
	void filterChanged(QString const & search);
	/// perform the search
	void search();
	/// reset the search box
	void resetSearch();
	/// set current panel from an item
	void switchPanel(QTreeWidgetItem * it, QTreeWidgetItem * previous = 0);
	/// click on the tree
	void itemSelected(QTreeWidgetItem *, int);

protected:
	/// widget hidden
	void hideEvent(QHideEvent * event) override;

private:
	///
	typedef QHash<QString, QTreeWidgetItem *> PanelMap;
	///
	PanelMap panel_map_;

	typedef QHash<QTreeWidgetItem *, QWidget *> WidgetMap;

	WidgetMap widget_map_;

	/// contains the search box
	FancyLineEdit * search_;

	/// contains the items
	QTreeWidget * list_;

	/// contains the panes
	QStackedWidget * stack_;

	// timer to delay the search between options
	QTimer * delay_search_;

};

} // namespace frontend
} // namespace lyx

#endif // PANELSTACK_H
