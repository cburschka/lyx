// -*- C++ -*-
/**
 * \file panelstack.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */
#ifndef PANELSTACK_H
#define PANELSTACK_H

#include "LString.h"

#include <qwidget.h>

#include <map>

class QListView;
class QListViewItem;
class QWidgetStack;

class PanelStack : public QWidget
{
	Q_OBJECT
public:
	PanelStack(QWidget * parent = 0, const char * name = "panelstack");

	/// add a category with no associated panel
	void addCategory(string const & name, string const & parent = string());

	/// add a widget panel with a given name, under the given parent
	void addPanel(QWidget * panel, string const & name, string const & parent = string());

	/// set current panel by logical name
	void setCurrentPanel(string const &);

public slots:
	/// set current panel from an item
	void switchPanel(QListViewItem * i);

private:
	typedef std::map<string, QListViewItem *> PanelMap;

	PanelMap panel_map_;

	typedef std::map<QListViewItem *, QWidget *> WidgetMap;

	WidgetMap widget_map_;

	/// contains the items
	QListView * list_;

	/// contains the panes
	QWidgetStack * stack_;
};

#endif // PANELSTACK_H
