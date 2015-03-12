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

#include "GuiApplication.h"
#include "qt_helpers.h"

#include "support/debug.h"
#include "support/foreach.h"
#include "support/lassert.h"

#include <QAbstractButton>
#include <QApplication>
#include <QComboBox>
#include <QFontMetrics>
#include <QGroupBox>
#include <QHideEvent>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPalette>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace std;

namespace lyx {
namespace frontend {


PanelStack::PanelStack(QWidget * parent)
	: QWidget(parent)
{
	delay_search_ = new QTimer(this);
	search_ = new FancyLineEdit(this);
	list_ = new QTreeWidget(this);
	stack_ = new QStackedWidget(this);

	// Configure the timer
	delay_search_->setSingleShot(true);
	connect(delay_search_, SIGNAL(timeout()), this, SLOT(search()));

	// Configure tree
	list_->setRootIsDecorated(false);
	list_->setColumnCount(1);
	list_->header()->hide();
	setSectionResizeMode(list_->header(), QHeaderView::ResizeToContents);
	list_->header()->setStretchLastSection(false);
	list_->setMinimumSize(list_->viewport()->size());

	connect(list_, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
		this, SLOT(switchPanel(QTreeWidgetItem *, QTreeWidgetItem*)));
	connect(list_, SIGNAL(itemClicked (QTreeWidgetItem*, int)),
		this, SLOT(itemSelected(QTreeWidgetItem *, int)));

	// Configure the search box
#if QT_VERSION >= 0x040700
	search_->setPlaceholderText(qt_("Search"));
#endif

#if QT_VERSION >= 0x040600
	search_->setButtonPixmap(FancyLineEdit::Right, getPixmap("images/", "editclear", "svgz,png"));
	search_->setButtonVisible(FancyLineEdit::Right, true);
	search_->setButtonToolTip(FancyLineEdit::Right, qt_("Clear text"));
	search_->setAutoHideButton(FancyLineEdit::Right, true);
#endif
	connect(search_, SIGNAL(rightButtonClicked()), this, SLOT(resetSearch()));
	connect(search_, SIGNAL(textEdited(QString)), this, SLOT(filterChanged(QString)));

	// Create the output layout, horizontal plus a VBox on the left with the search
	// box and the tree
	QVBoxLayout * left_layout = new QVBoxLayout;
	left_layout->addWidget(search_, 0);
	left_layout->addWidget(list_, 1);

	QHBoxLayout * main_layout = new QHBoxLayout(this);
	main_layout->addLayout(left_layout, 0);
	main_layout->addWidget(stack_, 1);
}


void PanelStack::addCategory(QString const & name, QString const & parent)
{
	QTreeWidgetItem * item = 0;

	LYXERR(Debug::GUI, "addCategory n= " << name << "   parent= ");

	int depth = 1;

	if (parent.isEmpty()) {
		item = new QTreeWidgetItem(list_);
		item->setText(0, qt_(name));
	}
	else {
		if (!panel_map_.contains(parent))
			addCategory(parent);
		item = new QTreeWidgetItem(panel_map_.value(parent));
		item->setText(0, qt_(name));
		depth = 2;
		list_->setRootIsDecorated(true);
	}

	panel_map_[name] = item;

	QFontMetrics fm(list_->font());
		
	// calculate the real size the current item needs in the listview
	int itemsize = fm.width(name) + 10 + list_->indentation() * depth;
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


void PanelStack::showPanel(QString const & name, bool show)
{
	QTreeWidgetItem * item = panel_map_.value(name, 0);
	LASSERT(item, return);

	item->setHidden(!show);
}


void PanelStack::setCurrentPanel(QString const & name)
{
	QTreeWidgetItem * item = panel_map_.value(name, 0);
	LASSERT(item, return);

	// force on first set
	if (list_->currentItem() == item)
		switchPanel(item);

	list_->setCurrentItem(item);
}


bool PanelStack::isCurrentPanel(QString const & name) const
{
	QTreeWidgetItem * item = panel_map_.value(name, 0);
	LASSERT(item, return false);

	return (list_->currentItem() == item);
}


void PanelStack::switchPanel(QTreeWidgetItem * item,
			     QTreeWidgetItem * previous)
{
	// do nothing when clicked on whitespace (item=NULL)
	if (!item)
		return;

	// if we have a category, expand the tree and go to the
	// first enabled item
	if (item->childCount() > 0) {
		item->setExpanded(true);
		if (previous && previous->parent() != item) {
			// Looks for a child not disabled
			for (int i = 0; i < item->childCount(); ++i) {
				if (item->child(i)->flags() & Qt::ItemIsEnabled) {
					switchPanel(item->child(i), previous);
					break;
				}
			}
		}
	}
	else if (QWidget * w = widget_map_.value(item, 0)) {
		stack_->setCurrentWidget(w);
	}
}

static bool matches(QString const & input, QString const & search)
{
	QString text = input;

	// Check if the input contains the search string
	return text.remove('&').contains(search, Qt::CaseInsensitive);
}

static void setTreeItemStatus(QTreeWidgetItem * tree_item, bool enabled)
{
	// Enable/disable the item
	tree_item->setDisabled(!enabled);

	// Change the color from black to gray or viceversa
	QPalette::ColorGroup new_color = enabled ? QPalette::Active : QPalette::Disabled;
	tree_item->setTextColor(0, QApplication::palette().color(new_color, QPalette::Text));
}

void PanelStack::hideEvent(QHideEvent * event)
{
	QWidget::hideEvent(event);

	// Programatically hidden (not simply minimized by the user)
	if (!event->spontaneous()) {
		resetSearch();
	}
}

void PanelStack::resetSearch()
{
	search_->setText(QString());
	search();
}

void PanelStack::filterChanged(QString const & /*search*/)
{
	// The text in the search box is changed, reset the timer
	// and then search in the widgets
	delay_search_->start(300);
}

void PanelStack::search()
{
	QString search = search_->text();
	bool enable_all = search.isEmpty();

	// If the search string is empty we enable all the items
	// otherwise we disable everything and then selectively
	// re-enable matching items
	foreach (QTreeWidgetItem * tree_item, panel_map_) {
		setTreeItemStatus(tree_item, enable_all);
	}

	foreach (QTreeWidgetItem * tree_item, panel_map_) {
		// Current widget
		QWidget * pane_widget = widget_map_[tree_item];

		// First of all we look in the pane name
		bool pane_matches = tree_item->text(0).contains(search, Qt::CaseInsensitive);

		// If the tree item has an associated pane
		if (pane_widget) {
			// Loops on the list of children widgets (recursive)
			QWidgetList children = pane_widget->findChildren<QWidget *>();
			foreach (QWidget * child_widget, children) {
				bool widget_matches = false;

				// Try to cast to the most common widgets and looks in it's content
				// It's bad OOP, it would be nice to have a QWidget::toString() overloaded by
				// each widget, but this would require to change Qt or subclass each widget.
				// Note that we have to ignore the amperstand symbol
				if (QAbstractButton * button = qobject_cast<QAbstractButton *>(child_widget)) {
					widget_matches = matches(button->text(), search);

				} else if (QGroupBox * group_box = qobject_cast<QGroupBox *>(child_widget)) {
					widget_matches = matches(group_box->title(), search);

				} else if (QLabel * label = qobject_cast<QLabel *>(child_widget)) {
					widget_matches = matches(label->text(), search);

				} else if (QLineEdit * line_edit = qobject_cast<QLineEdit *>(child_widget)) {
					widget_matches = matches(line_edit->text(), search);

				} else if (QListWidget * list_widget = qobject_cast<QListWidget *>(child_widget)) {
					widget_matches = (list_widget->findItems(search, Qt::MatchContains)).count() > 0;

				} else if (QTreeWidget * tree_view = qobject_cast<QTreeWidget *>(child_widget)) {
					widget_matches = (tree_view->findItems(search, Qt::MatchContains)).count() > 0;

				} else if (QComboBox * combo_box = qobject_cast<QComboBox *>(child_widget)) {
					widget_matches = (combo_box->findText(search, Qt::MatchContains)) != -1;

				} else {
					continue;
				}

				// If this widget meets the search criteria
				if (widget_matches && !enable_all) {
					// The pane too meets the search criteria
					pane_matches = true;

					// Highlight the widget
					QPalette widget_palette = child_widget->palette();
					widget_palette.setColor(child_widget->foregroundRole(), Qt::red);
					child_widget->setPalette(widget_palette);
				} else {
					// Reset the color of the widget
					child_widget->setPalette(QApplication::palette(child_widget));
				}
			}

			// If the pane meets the search criteria
			if (pane_matches && !enable_all) {
				// Expand and enable the pane and his ancestors	(typically just the parent)
				QTreeWidgetItem * item = tree_item;
				do {
					item->setExpanded(true);
					setTreeItemStatus(item, true);
					item = item->parent();
				} while (item);
			}
		}

	}
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

#include "moc_PanelStack.cpp"
