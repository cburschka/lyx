/*
 * tabstack.C
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#include "tabstack.h"

#include "qlayout.h"
#include "qwidgetstack.h"
#include "qtabbar.h"
#include "qpainter.h"

/**
 * A tab bar and a widget stack for storing related pages.
 */

TabStack::TabStack(QWidget * parent, const char * name)
	: QWidget(parent,name), tabs(0), stack(0), topLayout(0)
{
	stack = new QWidgetStack(this, "stack");
	tabs = new QTabBar(this, "tabbar");
	connect(tabs, SIGNAL(selected(int)), this, SLOT(selected(int)));
}

TabStack::~TabStack()
{
}

void TabStack::show()
{
	doLayout();
	QWidget::show();
}

void TabStack::doLayout()
{
	const int margin = 6;

	delete topLayout;
	topLayout = new QBoxLayout(this, QBoxLayout::Down);

	topLayout->addSpacing(margin);

	QBoxLayout * tmp = new QBoxLayout(QBoxLayout::LeftToRight);
	topLayout->addLayout(tmp, 0);
	tmp->addSpacing(margin);
	tmp->addWidget(tabs, 0);
	tmp->addStretch(1);
	tmp->addSpacing(margin + 2);

	tmp = new QBoxLayout(QBoxLayout::LeftToRight);
	topLayout->addLayout(tmp, 1);
	tmp->addSpacing(margin + 1);
	tmp->addWidget(stack, 1);
	tmp->addSpacing(margin + 2);

	topLayout->addSpacing(margin);

	topLayout->activate();
}

void TabStack::paintEvent(QPaintEvent *)
{
	if (!tabs)
		return;

	QPainter p;
	p.begin(this);

	QRect s(stack->geometry());

	QCOORD t = s.top() - 1;
	QCOORD b = s.bottom() + 2;
	QCOORD r = s.right() + 2;
	QCOORD l = s.left() - 1;

	p.setPen(colorGroup().light());
	p.drawLine(l, t, r - 1, t);
	p.drawLine(l, t + 1, l, b);
	p.setPen(black);
	p.drawLine(r, b, l,b);
	p.drawLine(r, b-1, r, t);
	p.setPen(colorGroup().dark());
	p.drawLine(l+1, b-1, r-1, b-1);
	p.drawLine(r-1, b-2, r-1, t+1);

	p.end();
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
