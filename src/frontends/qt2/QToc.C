/**
 * \file QToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <stack>

#include "LyXView.h"
#include "gettext.h"
#include "support/lstrings.h"
#include "debug.h"

#include "ControlToc.h"
#include "QToc.h"
#include "QTocDialog.h"
#include "Qt2BC.h"

#include <qslider.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qcombobox.h>

using std::endl;
using std::pair;
using std::stack;
using std::vector;

typedef Qt2CB<ControlToc, Qt2DB<QTocDialog> > base_class;

QToc::QToc()
	: base_class(_("Table of contents")), depth_(1)
{}


void QToc::build_dialog()
{
	dialog_.reset(new QTocDialog(this));

	// Manage the cancel/close button
	bc().setCancel(dialog_->closePB);
}


void QToc::updateType()
{
	dialog_->typeCO->clear();

	vector<string> const & choice = controller().getTypes();
	string const & type = toc::getType(controller().params().getCmdName());

	for (vector<string>::const_iterator it = choice.begin();
		it != choice.end(); ++it) {
		dialog_->typeCO->insertItem(it->c_str());
		if (*it == type) {
			dialog_->typeCO->setCurrentItem(it - choice.begin());
			dialog_->setCaption(type.c_str());
		}
	}
}


void QToc::update_contents()
{
	updateType();
	updateToc(depth_);
}


void QToc::updateToc(int newdepth)
{
	char const * str = dialog_->typeCO->currentText().latin1();
	string type (str ? str : "");

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
	stack< pair< QListViewItem *, QListViewItem *> > istack;
	QListViewItem *last = 0;
	QListViewItem *parent = 0;
	QListViewItem *item;

	// Yes, it is this ugly. Two reasons - root items must have a QListView parent,
	// rather than QListViewItem; and the TOC can move in and out an arbitrary number
	// of levels

	for (toc::Toc::const_iterator iter = toclist.begin();
		iter != toclist.end(); ++iter) {
		if (iter->depth == curdepth) {
			// insert it after the last one we processed
			if (!parent)
				item = (last) ? (new QListViewItem(dialog_->tocLV,last)) : (new QListViewItem(dialog_->tocLV));
			else
				item = (last) ? (new QListViewItem(parent,last)) : (new QListViewItem(parent));
		} else if (iter->depth > curdepth) {
			int diff = iter->depth - curdepth;
			// first save old parent and last
			while (diff--)
				istack.push(pair< QListViewItem *, QListViewItem * >(parent,last));
			item = (last) ? (new QListViewItem(last)) : (new QListViewItem(dialog_->tocLV));
			parent = last;
		} else {
			int diff = curdepth - iter->depth;
			pair< QListViewItem *, QListViewItem * > top;
			// restore context
			while (diff--) {
				top = istack.top();
				istack.pop();
			}
			parent = top.first;
			last = top.second;
			// insert it after the last one we processed
			if (!parent)
				item = (last) ? (new QListViewItem(dialog_->tocLV,last)) : (new QListViewItem(dialog_->tocLV));
			else
				item = (last) ? (new QListViewItem(parent,last)) : (new QListViewItem(parent));
		}

		lyxerr[Debug::GUI] << "Table of contents" << endl << "Added item " << iter->str.c_str()
			<< " at depth " << iter->depth << ", previous sibling \"" << (last ? last->text(0).latin1() : "0")
			<< "\", parent \"" << (parent ? parent->text(0).latin1() : "0") << "\"" << endl;
		item->setText(0,iter->str.c_str());
		item->setOpen(iter->depth < depth_);
		curdepth = iter->depth;
		last = item;
	}

	dialog_->tocLV->setUpdatesEnabled(true);
	dialog_->tocLV->update();
}


void QToc::select(string const & text)
{
	toc::Toc::const_iterator iter = toclist.begin();

	for (; iter != toclist.end(); ++iter) {
		if (iter->str == text)
			break;
	}

	if (iter == toclist.end()) {
		lyxerr[Debug::GUI] << "Couldn't find highlighted TOC entry : "
			<< text << endl;
		return;
	}

	controller().goTo(*iter);
}


void QToc::set_depth(int depth)
{
	if (depth != depth_)
		updateToc(depth);
}
