/**
 * \file panelstack.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include "panelstack.h"

#include "support/LAssert.h"

#include "qt_helpers.h"

#include <qwidgetstack.h>
#include <qlayout.h>
#include <qlistview.h>

using std::map;


PanelStack::PanelStack(QWidget * parent, const char * name)
	: QWidget(parent, name)
{
	list_ = new QListView(this);
	stack_ = new QWidgetStack(this);
	list_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	stack_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	list_->setSorting(-1);
	list_->setHScrollBarMode(QScrollView::AlwaysOff);
	list_->setVScrollBarMode(QScrollView::AlwaysOff);
	list_->addColumn("");
	list_->setColumnWidthMode(0, QListView::Maximum);
	list_->setResizeMode(QListView::AllColumns);
	list_->setRootIsDecorated(true);

	connect(list_, SIGNAL(currentChanged(QListViewItem*)),
	        this, SLOT(switchPanel(QListViewItem *)));

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->addWidget(list_, 0);
	layout->addWidget(stack_, 1);
}


void PanelStack::addCategory(string const & n, string const & parent)
{
	QListViewItem * item;

	QString name = toqstr(n);

	if (!parent.empty()) {
		PanelMap::iterator it = panel_map_.find(parent);
		lyx::Assert(it != panel_map_.end());

		QListViewItem * before = it->second->firstChild();
		if (before) {
			while (before->nextSibling())
				before = before->nextSibling();

			item = new QListViewItem(it->second, before, name);
		} else {
			item = new QListViewItem(it->second, name);
		}
	} else {
		QListViewItem * before = list_->firstChild();
		if (before) {
			while (before->nextSibling())
				before = before->nextSibling();
			item = new QListViewItem(list_, before, name);
		} else {
			item = new QListViewItem(list_, name);
		}
	}

	item->setSelectable(false);
	item->setOpen(true);
	panel_map_[n] = item;

	// Qt is just unbelievably moronic
	list_->setMinimumSize(QSize(150, list_->minimumHeight()));
}


void PanelStack::addPanel(QWidget * panel, string const & name, string const & parent)
{
	addCategory(name, parent);
	QListViewItem * item = panel_map_.find(name)->second;

	// reset the selectability set by addCategory
	item->setSelectable(true);

	widget_map_[item] = panel;
	stack_->addWidget(panel);
	stack_->setMinimumSize(panel->minimumSize());
	resize(sizeHint());
}


void PanelStack::setCurrentPanel(string const & name)
{
	PanelMap::const_iterator cit = panel_map_.find(name);
	lyx::Assert(cit != panel_map_.end());

	switchPanel(cit->second);
}


void PanelStack::switchPanel(QListViewItem * item)
{
	WidgetMap::const_iterator cit = widget_map_.find(item);
	if (cit == widget_map_.end())
		return;

	stack_->raiseWidget(cit->second);
}
