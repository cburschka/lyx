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

#include <QWidget>

#include <map>
#include <string>

class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;

class PanelStack : public QWidget
{
	Q_OBJECT
public:
	PanelStack(QWidget * parent = 0);

	/// add a category with no associated panel
	void addCategory(std::string const & name, std::string const & parent = std::string());

	/// add a widget panel with a given name, under the given parent
	void addPanel(QWidget * panel, std::string const & name, std::string const & parent = std::string());

	/// set current panel by logical name
	void setCurrentPanel(std::string const &);

public slots:
	/// set current panel from an item
	void switchPanel(QTreeWidgetItem * i, QTreeWidgetItem* previous=0);

private:
	typedef std::map<std::string, QTreeWidgetItem *> PanelMap;

	PanelMap panel_map_;

	typedef std::map<QTreeWidgetItem *, QWidget *> WidgetMap;

	WidgetMap widget_map_;

	/// contains the items
	QTreeWidget * list_;

	/// contains the panes
	QStackedWidget * stack_;
};

#endif // PANELSTACK_H
