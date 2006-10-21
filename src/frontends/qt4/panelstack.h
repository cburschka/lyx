// -*- C++ -*-
/**
 * \file panelstack.h
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

#include "support/docstring.h"

#include <QWidget>

#include <map>
#include <string>


class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;

//namespace lyx {

class PanelStack : public QWidget
{
	Q_OBJECT
public:
	PanelStack(QWidget * parent = 0);

	/// add a category with no associated panel
	void addCategory(lyx::docstring const & name,
		lyx::docstring const & parent = lyx::docstring());

	/// add a widget panel with a given name, under the given parent
	void addPanel(QWidget * panel, lyx::docstring const & name,
		lyx::docstring const & parent = lyx::docstring());

	/// set current panel by logical name
	void setCurrentPanel(lyx::docstring const &);

public Q_SLOTS:
	/// set current panel from an item
	void switchPanel(QTreeWidgetItem * i, QTreeWidgetItem* previous=0);

private:
	typedef std::map<lyx::docstring, QTreeWidgetItem *> PanelMap;

	PanelMap panel_map_;

	typedef std::map<QTreeWidgetItem *, QWidget *> WidgetMap;

	WidgetMap widget_map_;

	/// contains the items
	QTreeWidget * list_;

	/// contains the panes
	QStackedWidget * stack_;
};


//} // namespace lyx

#endif // PANELSTACK_H
