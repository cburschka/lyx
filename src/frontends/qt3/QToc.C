/**
 * \file QToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QToc.h"
#include "QTocDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlToc.h"

#include <qcombobox.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <stack>

using std::endl;

using std::pair;
using std::stack;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlToc, QView<QTocDialog> > base_class;

QToc::QToc(Dialog & parent)
	: base_class(parent, _("Table of Contents")), depth_(1)
{}


void QToc::build_dialog()
{
	dialog_.reset(new QTocDialog(this));

	// Manage the cancel/close button
	bcview().setCancel(dialog_->closePB);
	type_ = toc::getType(controller().params().getCmdName());
	dialog_->enableButtons();
}


void QToc::updateType()
{
	dialog_->typeCO->clear();

	vector<string> const & choice = controller().getTypes();
	string const & type = toc::getType(controller().params().getCmdName());

	for (vector<string>::const_iterator it = choice.begin();
		it != choice.end(); ++it) {
		string const & guiname = controller().getGuiName(*it);
		dialog_->typeCO->insertItem(toqstr(guiname));
		if (*it == type) {
			dialog_->typeCO->setCurrentItem(it - choice.begin());
			setTitle(lyx::from_ascii(guiname));
		}
	}
	type_ = type;
	dialog_->enableButtons();
}


void QToc::update_contents()
{
	updateType();
	updateToc(depth_);
}


void QToc::updateToc(int newdepth)
{
	vector<string> const & choice = controller().getTypes();
	string type;
	if (!choice.empty())
		type = choice[dialog_->typeCO->currentItem()];
	type_ = type;
	dialog_->enableButtons();

	toc::Toc const & contents = controller().getContents(type);

	// Check if all elements are the same.
	if (newdepth == depth_ && toclist == contents) {
		return;
	}

	dialog_->tocLV->clear();

	depth_ = newdepth;

	toclist = contents;

	if (toclist.empty())
		return;

	dialog_->tocLV->setUpdatesEnabled(false);

	int curdepth = 0;
	stack<pair<QListViewItem *, QListViewItem *> > istack;
	QListViewItem * last = 0;
	QListViewItem * parent = 0;
	QListViewItem * item;
	QListViewItem * selected_item = 0;
	bool multiple = false;

	// Yes, it is this ugly. Two reasons - root items must have
	// a QListView parent, rather than QListViewItem; and the
	// TOC can move in and out an arbitrary number of levels

	for (toc::Toc::const_iterator iter = toclist.begin();
	     iter != toclist.end(); ++iter) {
		if (iter->depth() == curdepth) {
			// insert it after the last one we processed
			if (!parent)
				item = (last ? new QListViewItem(dialog_->tocLV,last) : new QListViewItem(dialog_->tocLV));
			else
				item = (last ? new QListViewItem(parent,last) : new QListViewItem(parent));
		} else if (iter->depth() > curdepth) {
			int diff = iter->depth() - curdepth;
			// first save old parent and last
			while (diff--)
				istack.push(pair< QListViewItem *, QListViewItem * >(parent,last));
			item = (last ? new QListViewItem(last) : new QListViewItem(dialog_->tocLV));
			parent = last;
		} else {
			int diff = curdepth - iter->depth();
			pair<QListViewItem *, QListViewItem * > top;
			// restore context
			while (diff--) {
				top = istack.top();
				istack.pop();
			}
			parent = top.first;
			last = top.second;
			// insert it after the last one we processed
			if (!parent)
				item = (last ? new QListViewItem(dialog_->tocLV,last) : new QListViewItem(dialog_->tocLV));
			else
				item = (last ? new QListViewItem(parent,last) : new QListViewItem(parent));
		}

		lyxerr[Debug::GUI]
			<< "Table of contents\n"
			<< "Added item " << iter->str()
			<< " at depth " << iter->depth()
			<< ", previous sibling \""
			<< (last ? fromqstr(last->text(0)) : "0")
			<< "\", parent \""
			<< (parent ? fromqstr(parent->text(0)) : "0") << '"'
			<< endl;
		item->setText(0, toqstr(iter->str()));
		item->setOpen(iter->depth() < depth_);
		curdepth = iter->depth();
		last = item;

		// Recognise part past the counter
		if (iter->str().substr(iter->str().find(' ') + 1) == text_) {
			if (selected_item == 0)
				selected_item = item;
			else
				// more than one match
				multiple = true;
		}
	}

	dialog_->tocLV->setUpdatesEnabled(true);
	dialog_->tocLV->update();
	if (!multiple) {
		dialog_->tocLV->scrollBy(0, selected_item->itemPos()
			- dialog_->tocLV->height() / 2);
		dialog_->tocLV->setSelected(selected_item, true);
	}
	setTitle(qstring_to_ucs4(dialog_->typeCO->currentText()));
}


bool QToc::canOutline()
{
	return controller().canOutline(type_);
}


void QToc::select(string const & text)
{
	toc::Toc::const_iterator iter = toclist.begin();

	for (; iter != toclist.end(); ++iter) {
		if (iter->str() == text)
			break;
	}

	if (iter == toclist.end()) {
		lyxerr[Debug::GUI] << "Couldn't find highlighted TOC entry: "
			<< text << endl;
		return;
	}

	// Lop off counter part and save:
	text_ = text.substr(text.find(' ') + 1);
	controller().goTo(*iter);
}


void QToc::set_depth(int depth)
{
	if (depth != depth_)
		updateToc(depth);
}


void QToc::moveup()
{
	controller().outlineUp();
	updateToc(depth_);
}


void QToc::movedn()
{
	controller().outlineDown();
	updateToc(depth_);
}


void QToc::movein()
{
	controller().outlineIn();
	updateToc(depth_);
}


void QToc::moveout()
{
	controller().outlineOut();
	updateToc(depth_);
}


} // namespace frontend
} // namespace lyx
