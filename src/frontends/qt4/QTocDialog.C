/**
 * \file QTocDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTocDialog.h"
#include "QToc.h"
#include "qt_helpers.h"
#include "controllers/ControlToc.h"

#include "debug.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QCloseEvent>

#include <vector>
#include <string>
#include <stack>

using std::endl;
using std::pair;
using std::stack;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

QTocDialog::QTocDialog(QToc * form)
	: form_(form), depth_(2)
{
	setupUi(this);

	// disable sorting
	tocTW->setSortingEnabled(false);
	tocTW->setColumnCount(1);
	// hide the pointless QHeader
//	QWidget * w = static_cast<QWidget*>(tocTW->child("list view header"));
//	if (w)
//		w->hide();

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

	connect( tocTW, SIGNAL(itemClicked(QTreeWidgetItem*)), this, SLOT(select_adaptor(QTreeWidgetItem*)));
    connect( typeCO, SIGNAL( activated(int) ), this, SLOT( activate_adaptor(int) ) );
    connect( updatePB, SIGNAL( clicked() ), this, SLOT( update_adaptor() ) );
    connect( depthSL, SIGNAL( valueChanged(int) ), this, SLOT( depth_adaptor(int) ) );
}


QTocDialog::~QTocDialog()
{
}

void QTocDialog::updateType()
{
	typeCO->clear();

	vector<string> const & choice = form_->controller().getTypes();
	string const & type = toc::getType(form_->controller().params().getCmdName());

	for (vector<string>::const_iterator it = choice.begin();
		it != choice.end(); ++it) {
		string const & guiname = form_->controller().getGuiName(*it);
		typeCO->insertItem(toqstr(guiname));
		if (*it == type) {
			typeCO->setCurrentItem(it - choice.begin());
			form_->setTitle(guiname);
		}
	}
}

void QTocDialog::updateToc(bool newdepth)
{
	vector<string> const & choice = form_->controller().getTypes();
	string type;
	if (!choice.empty())
		type = choice[typeCO->currentItem()];

	toc::Toc const & contents = form_->controller().getContents(type);

	// Check if all elements are the same.
	if (!newdepth && form_->get_toclist() == contents) {
		return;
	}

	tocTW->clear();

	form_->get_toclist() = contents;

	if (form_->get_toclist().empty())
		return;

	tocTW->setUpdatesEnabled(false);

	QTreeWidgetItem * topLevelItem;

	toc::Toc::const_iterator iter = form_->get_toclist().begin();

	while (iter != form_->get_toclist().end()) {

		if (iter->depth == 1) {
			topLevelItem = new QTreeWidgetItem(tocTW);
//			tocTW->addTopLevelItem(topLevelItem);
			topLevelItem->setText(0, toqstr(iter->str));
			if (iter->depth < depth_) tocTW->collapseItem(topLevelItem);

			lyxerr[Debug::GUI]
				<< "Table of contents\n"
				<< "Added Top Level item " << iter->str
				<< " at depth " << iter->depth
				<< endl;

			populateItem(topLevelItem, iter);
		}

		if (iter == form_->get_toclist().end())
			break;

		++iter;
	}

	tocTW->setUpdatesEnabled(true);
	tocTW->update();
	form_->setTitle(fromqstr(typeCO->currentText()));
	tocTW->show();
}

void QTocDialog::populateItem(QTreeWidgetItem * parentItem, toc::Toc::const_iterator& iter)
{
	int curdepth = iter->depth+1;
	QTreeWidgetItem * item;

	while (iter != form_->get_toclist().end()) {
		
		++iter;

		if (iter == form_->get_toclist().end())
			break;

		if (iter->depth < curdepth) {
			--iter;
			return;
		}
		if (iter->depth > curdepth) {
//			--iter;
			return;
		}

		item = new QTreeWidgetItem(parentItem);
		item->setText(0, toqstr(iter->str));
//		parentItem->addChild(item);

		if (iter->depth < depth_) tocTW->collapseItem(item);
//		else tocTW->expandItem(item);
		lyxerr[Debug::GUI]
			<< "Table of contents: Added item " << iter->str
			<< " at depth " << iter->depth
			<< "  \", parent \""
			<< fromqstr(parentItem->text(0)) << '"'
			<< endl;

		populateItem(item, iter);
	}
}

void QTocDialog::activate_adaptor(int)
{
	updateToc();
}


void QTocDialog::depth_adaptor(int depth)
{
	if (depth == depth_)
		return;

	depth_ = depth;
	updateToc(true);
}


void QTocDialog::select_adaptor(QTreeWidgetItem * item)
{
	form_->select(fromqstr(item->text(0)));
}


void QTocDialog::update_adaptor()
{
	form_->update();
}


void QTocDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx


/*
	stack<pair<QTreeWidgetItem *, QTreeWidgetItem *> > istack;
	QTreeWidgetItem * last = 0;
	QTreeWidgetItem * parent = 0;
	QTreeWidgetItem * item;

	// Yes, it is this ugly. Two reasons - root items must have
	// a QListView parent, rather than QListViewItem; and the
	// TOC can move in and out an arbitrary number of levels

	for (toc::Toc::const_iterator iter = form_->get_toclist().begin();
	     iter != form_->get_toclist().end(); ++iter) {

		if (iter->depth == 0) {
			TocTW

		if (iter->depth == curdepth) {
			// insert it after the last one we processed
			if (!parent)
				item = (last ? new QTreeWidgetItem(tocTW,last) : new QTreeWidgetItem(tocTW));
			else
				item = (last ? new QTreeWidgetItem(parent,last) : new QTreeWidgetItem(parent));
		} else if (iter->depth > curdepth) {
			int diff = iter->depth - curdepth;
			// first save old parent and last
			while (diff--)
				istack.push(pair< QTreeWidgetItem *, QTreeWidgetItem * >(parent,last));
			item = (last ? new QTreeWidgetItem(last) : new QTreeWidgetItem(tocTW));
			parent = last;
		} else {
			int diff = curdepth - iter->depth;
			pair<QTreeWidgetItem *, QTreeWidgetItem * > top;
			// restore context
			while (diff--) {
				top = istack.top();
				istack.pop();
			}
			parent = top.first;
			last = top.second;
			// insert it after the last one we processed
			if (!parent)
				item = (last ? new QTreeWidgetItem(tocTW,last) : new QTreeWidgetItem(tocTW));
			else
				item = (last ? new QTreeWidgetItem(parent,last) : new QTreeWidgetItem(parent));
		}
		item->setText(0, toqstr(iter->str));
		item->setOpen(iter->depth < depth_);
		curdepth = iter->depth;
		last = item;
	}
*/