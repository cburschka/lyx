// -*- C++ -*-
/*
 * tabstack.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TABSTACK_H
#define TABSTACK_H

#include <qwidget.h>

class QTabBar;
class QWidgetStack; 
class QBoxLayout;

#include "boost/utility.hpp"

/**
 * This widget provides a tab bar which can switch between a stack
 * of QFrames.
 */
class TabStack : public QWidget, boost::noncopyable {
   Q_OBJECT
public:
	TabStack(QWidget * parent = 0, const char * name = 0);
	
	~TabStack();

	/// add a page widget to the stack
	virtual int addTabPage(QWidget *, const char *);
	/// set the enabled status of a tab page
	virtual void setTabPageEnabled(int, bool);
	/// is a tab page enabled ?
	virtual bool isTabPageEnabled(int) const;
	/// raise the given tab page
	virtual void setCurrentTabPage(int);
	/// which tab page is currently on top
	virtual int currentTabPage(void) const;

public slots:
	/// show the widget 
	virtual void show(void);

protected slots: 
	/// a tab page has been selected 
	virtual void selected(int);

	/// paint widget
	virtual void paintEvent(QPaintEvent *);
 
private:
	/// set up layout
	void doLayout(void);

	QTabBar *tabs;
	QWidgetStack *stack;
	QBoxLayout *topLayout;
};

#endif
