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

#include <QWidget>
#include <QHash>

class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;

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
	/// set current panel by logical name
	void setCurrentPanel(QString const &);
	///
	QSize sizeHint() const;

public Q_SLOTS:
	/// set current panel from an item
	void switchPanel(QTreeWidgetItem * it, QTreeWidgetItem * previous = 0);
	/// click on the tree
	void itemSelected(QTreeWidgetItem *, int);

private:
	///
	typedef QHash<QString, QTreeWidgetItem *> PanelMap;
	///
	PanelMap panel_map_;

	typedef QHash<QTreeWidgetItem *, QWidget *> WidgetMap;

	WidgetMap widget_map_;

	/// contains the items
	QTreeWidget * list_;

	/// contains the panes
	QStackedWidget * stack_;
};

} // namespace frontend
} // namespace lyx

#endif // PANELSTACK_H
