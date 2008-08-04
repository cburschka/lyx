/**
 * \file PanelStack.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PanelStack.h"

#include "qt_helpers.h"

#include "support/debug.h"

#include <QFontMetrics>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include "support/lassert.h"

#include <iostream>

using namespace std;

namespace lyx {
namespace frontend {


PanelStack::PanelStack(QWidget * parent)
	: QWidget(parent)
{
	list_ = new QTreeWidget(this);
	stack_ = new QStackedWidget(this);

	list_->setRootIsDecorated(false);
	list_->setColumnCount(1);
	// Hide the pointless list header
	list_->header()->hide();
//	QStringList HeaderLabels;
//	HeaderLabels << QString("Category");
//	list_->setHeaderLabels(HeaderLabels);

	connect(list_, SIGNAL(currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(switchPanel(QTreeWidgetItem *, QTreeWidgetItem*)));
	connect(list_, SIGNAL(itemClicked (QTreeWidgetItem*, int)),
		this, SLOT(itemSelected(QTreeWidgetItem *, int)));

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->addWidget(list_, 0);
	layout->addWidget(stack_, 1);
}


void PanelStack::addCategory(QString const & name, QString const & parent)
{
	QTreeWidgetItem * item = 0;

	LYXERR(Debug::GUI, "addCategory n= " << name << "   parent= ");

	int depth = 1;

	if (parent.isEmpty()) {
		item = new QTreeWidgetItem(list_);
		item->setText(0, name);
	}
	else {
		if (!panel_map_.contains(parent))
			addCategory(parent);
		item = new QTreeWidgetItem(panel_map_.value(parent));
		item->setText(0, name);
		depth = 2;
		list_->setRootIsDecorated(true);
	}

	panel_map_[name] = item;

	QFontMetrics fm(list_->font());
		
	// calculate the real size the current item needs in the listview
	int itemsize = fm.width(name) + 10
		+ list_->indentation() * depth;
	// adjust the listview width to the max. itemsize
	if (itemsize > list_->minimumWidth())
		list_->setMinimumWidth(itemsize);
}


void PanelStack::addPanel(QWidget * panel, QString const & name, QString const & parent)
{
	addCategory(name, parent);
	QTreeWidgetItem * item = panel_map_.value(name);
	widget_map_[item] = panel;
	stack_->addWidget(panel);
	stack_->setMinimumSize(panel->minimumSize());
}


void PanelStack::setCurrentPanel(QString const & name)
{
	QTreeWidgetItem * item = panel_map_.value(name, 0);
	LASSERT(item, /**/);

	// force on first set
	if (list_->currentItem() == item)
		switchPanel(item);

	list_->setCurrentItem(item);
}


void PanelStack::switchPanel(QTreeWidgetItem * item,
			     QTreeWidgetItem * previous)
{
	// do nothing when clicked on whitespace (item=NULL)
	if( !item )
		return;

	// if we have a category, expand the tree and go to the
	// first item
	if (item->childCount() > 0) {
		item->setExpanded(true);
		if (previous && previous->parent() != item)
			switchPanel( item->child(0), previous );
	}
	else
		if (QWidget * w = widget_map_.value(item, 0))
			stack_->setCurrentWidget(w);
}


void PanelStack::itemSelected(QTreeWidgetItem * item, int)
{
	// de-select the category if a child is selected
	if (item->childCount() > 0 && item->child(0)->isSelected())
		item->setSelected(false);
}


QSize PanelStack::sizeHint() const
{
	return QSize(list_->width() + stack_->width(),
		qMax(list_->height(), stack_->height()));
}

} // namespace frontend
} // namespace lyx

#include "PanelStack_moc.cpp"
