/**
 * \file panelstack.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "panelstack.h"

#include "qt_helpers.h"

#include <QStackedWidget>
#include <QFontMetrics>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QLayout>

#include <boost/assert.hpp>

using std::string;

#include <iostream>
using std::endl;
using std::cout;


PanelStack::PanelStack(QWidget * parent)
	: QWidget(parent)
{
	list_ = new QTreeWidget(this);
	stack_ = new QStackedWidget(this);

	list_->setColumnCount(1);
	list_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	stack_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	list_->setSortingEnabled(false);
//	list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//	list_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//	list_->addColumn("");
//	list_->setColumnWidthMode(0, QTreeWidget::Maximum);

//	list_->setResizeMode(QTreeWidget::AllColumns);

	QStringList HeaderLabels; HeaderLabels << QString("Category");
	list_->setHeaderLabels(HeaderLabels);

	connect(list_, SIGNAL(currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(switchPanel(QTreeWidgetItem *, QTreeWidgetItem*)));

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->addWidget(list_, 0);
	layout->addWidget(stack_, 1);
}


void PanelStack::addCategory(string const & n, string const & parent)
{
	QTreeWidgetItem * item;

	QString name = toqstr(n);

	cout << "addCategory n= " << n << "   parent= " << endl;

	if (parent.empty()) {
		item = new QTreeWidgetItem(list_);
		item->setText(0, name);
		//list_->addTopLevelItem(item);
	}
	else {
		PanelMap::iterator it = panel_map_.find(parent);
		BOOST_ASSERT(it != panel_map_.end());
		item = new QTreeWidgetItem(it->second);
		item->setText(0, name);
		//it->second->addChild(item);
	}

	panel_map_[n] = item;

	list_->setFixedWidth(list_->sizeHint().width());
/*
	item->setFlags(false);
	item->setOpen(true);

	// calculate the real size the current item needs in the listview
	int itemsize = item->width(list_->fontMetrics(), list_, 0) + 10
		   + list_->treeStepSize() * (item->depth() + 1) + list_->itemMargin();
	// adjust the listview width to the max. itemsize
	if (itemsize > list_->minimumWidth())
		list_->setMinimumWidth(itemsize);
		*/
}


void PanelStack::addPanel(QWidget * panel, string const & name, string const & parent)
{
	addCategory(name, parent);
	QTreeWidgetItem * item = panel_map_.find(name)->second;

	// reset the selectability set by addCategory
//	item->setSelectable(true);

	widget_map_[item] = panel;
	stack_->addWidget(panel);
	stack_->setMinimumSize(panel->minimumSize());
}


void PanelStack::setCurrentPanel(string const & name)
{
	PanelMap::const_iterator cit = panel_map_.find(name);
	BOOST_ASSERT(cit != panel_map_.end());

	// force on first set
	if (list_->currentItem() ==  cit->second)
		switchPanel(cit->second);

	list_->setCurrentItem(cit->second);
}


void PanelStack::switchPanel(QTreeWidgetItem * item, QTreeWidgetItem* previous)
{
	WidgetMap::const_iterator cit = widget_map_.find(item);
	if (cit == widget_map_.end())
		return;

	stack_->setCurrentWidget(cit->second);
}

#include "panelstack_moc.cpp"
