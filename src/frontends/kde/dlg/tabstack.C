/*
 * tabstack.C
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

#include "tabstack.h"

#include "qlayout.h" 
#include "qwidgetstack.h"
#include "qtabbar.h"
#include "qpainter.h"

/**
 * A tab bar and a widget stack for storing related pages.
 */

TabStack::TabStack(QWidget * parent = 0, const char * name = 0)
	: QWidget(parent,name), tabs(0), stack(0)
{
	stack = new QWidgetStack(this, "stack");
	tabs = new QTabBar(this, "tabbar");
	connect(tabs, SIGNAL(selected(int)), this, SLOT(selected(int)));
	
	topLayout = new QHBoxLayout(this, 1);

	layout = new QVBoxLayout();
	topLayout->addLayout(layout);
	layout->addWidget(tabs, 0);
	layout->addWidget(stack, 1);
}

TabStack::~TabStack()
{
}

int TabStack::addTabPage(QWidget *page, const char *label)
{
	QTab *tab = new QTab();

	tab->label = label;

	int id = tabs->addTab(tab);
	stack->addWidget(page, id);
	tabs->setMinimumSize(tabs->sizeHint());
	return id;
}

void TabStack::setTabPageEnabled(int id, bool enable)
{
	tabs->setTabEnabled(id, enable);
}

bool TabStack::isTabPageEnabled(int id) const
{
	return tabs->isTabEnabled(id);
} 

void TabStack::setCurrentTabPage(int id)
{
	selected(id);
}

int TabStack::currentTabPage() const
{
	return tabs->currentTab();
}

void TabStack::selected(int id)
{
	if (tabs->currentTab() != id)
		tabs->setCurrentTab(id);
	stack->raiseWidget(id);
}

void TabStack::paintEvent(QPaintEvent *)
{
	if (!tabs)
		return;
	
	QPainter p;
	p.begin(this);
	QRect geom(stack->geometry());
	 
	QCOORD top = geom.top() - 1;
	QCOORD bottom = geom.bottom() + 2;
	QCOORD right = geom.right() + 2;
	QCOORD left = geom.left() - 1;

	p.setPen(white);
	p.drawLine(left, top, right - 1, top);
	p.drawLine(left, top + 1, left, bottom);
	p.setPen(black);
	p.drawLine(right, bottom, left, bottom);
	p.drawLine(right, bottom-1, right, top);
	p.setPen(colorGroup().dark());
	p.drawLine(left+1, bottom-1, right-1, bottom-1);
	p.drawLine(right-1, bottom-2, right-1, top+1);

	p.end(); 
	
	// FIXME: do this better ?
	tabs->update();
}
