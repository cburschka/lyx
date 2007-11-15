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

#include "debug.h"

#include <QFontMetrics>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QHeaderView>

#include <boost/assert.hpp>

#include <iostream>


using std::endl;
using std::cout;

namespace lyx {
namespace frontend {


PanelStack::PanelStack(QWidget * parent)
	: QWidget(parent)
{
	list_ = new QTreeWidget(this);
	stack_ = new QStackedWidget(this);

	list_->setColumnCount(1);
	list_->setRootIsDecorated(false);
	// Hide the pointless list header
	list_->header()->hide();
//	QStringList HeaderLabels;
//	HeaderLabels << QString("Category");
//	list_->setHeaderLabels(HeaderLabels);

	connect(list_, SIGNAL(currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(switchPanel(QTreeWidgetItem *, QTreeWidgetItem*)));

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->addWidget(list_, 0);
	layout->addWidget(stack_, 1);
}


void PanelStack::addCategory(docstring const & n, docstring const & parent)
{
	QTreeWidgetItem * item = 0;
	QString const name = toqstr(n);

	LYXERR(Debug::GUI, "addCategory n= " << to_utf8(n) << "   parent= ");

	int depth = 1;

	if (parent.empty()) {
		item = new QTreeWidgetItem(list_);
		item->setText(0, name);
	}
	else {
		PanelMap::iterator it = panel_map_.find(parent);
		//BOOST_ASSERT(it != panel_map_.end());
		if (it == panel_map_.end()) {
			addCategory(parent);
			it = panel_map_.find(parent);
		}
		BOOST_ASSERT(it != panel_map_.end());

		item = new QTreeWidgetItem(it->second);
		item->setText(0, name);
		depth = 2;
	}

	panel_map_[n] = item;

	QFontMetrics fm(list_->font());
	// calculate the real size the current item needs in the listview
	int itemsize = fm.width(name) + 10
		+ list_->indentation() * depth;
	// adjust the listview width to the max. itemsize
	if (itemsize > list_->minimumWidth())
		list_->setMinimumWidth(itemsize);
}


void PanelStack::addPanel(QWidget * panel, docstring const & name, docstring const & parent)
{
	addCategory(name, parent);
	QTreeWidgetItem * item = panel_map_.find(name)->second;

	widget_map_[item] = panel;
	stack_->addWidget(panel);
	stack_->setMinimumSize(panel->minimumSize());
}


void PanelStack::setCurrentPanel(docstring const & name)
{
	PanelMap::const_iterator cit = panel_map_.find(name);
	BOOST_ASSERT(cit != panel_map_.end());

	// force on first set
	if (list_->currentItem() ==  cit->second)
		switchPanel(cit->second);

	list_->setCurrentItem(cit->second);
}


void PanelStack::switchPanel(QTreeWidgetItem * item,
			     QTreeWidgetItem * /*previous*/)
{
	WidgetMap::const_iterator cit = widget_map_.find(item);
	if (cit == widget_map_.end())
		return;

	stack_->setCurrentWidget(cit->second);
}


QSize PanelStack::sizeHint() const
{
	return QSize(list_->width() + stack_->width(),
		qMax(list_->height(), stack_->height()));
}

} // namespace frontend
} // namespace lyx

#include "PanelStack_moc.cpp"
