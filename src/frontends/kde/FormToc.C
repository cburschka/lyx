/*
 * FormToc.C
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

#include <config.h>

#include <stack>

#include "tocdlg.h"

#include "Dialogs.h"
#include "FormToc.h"
#include "gettext.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "QtLyXView.h"
#include "lyxfunc.h"
#include "debug.h"

using std::vector;
using std::pair;
using std::stack;
using std::endl;
 
FormToc::FormToc(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0),
	toclist(0), type(Buffer::TOC_TOC), depth(1)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTOC.connect(slot(this, &FormToc::showTOC));
	d->createTOC.connect(slot(this, &FormToc::createTOC));
}

FormToc::~FormToc()
{
	delete dialog_;
}

void FormToc::showTOC(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	ih_ = inset_->hide.connect(slot(this,&FormToc::hide));
	params = inset->params();
	
	show();
}

void FormToc::createTOC(string const & arg)
{
	if (inset_)
		close();

	params.setFromString(arg);
	show();
}

void FormToc::updateToc(int newdepth)
{
	if (!lv_->view()->available()) {
		toclist.clear();
		dialog_->tree->clear();
		return;
	}

	vector< vector<Buffer::TocItem> > tmp =
		lv_->view()->buffer()->getTocList();

	// Check if all elements are the same.
	if (newdepth==depth && toclist.size() == tmp[type].size()) {
		unsigned int i = 0;
		for (; i < toclist.size(); ++i) {
			if (toclist[i] !=  tmp[type][i])
				break;
		}
		if (i >= toclist.size())
			return;
	}

	depth=newdepth;

	toclist = tmp[type];

	dialog_->tree->clear();
	if (toclist.empty()) 
		return;

	dialog_->tree->setUpdatesEnabled(false);

	int curdepth = 0;
	stack< pair< QListViewItem *, QListViewItem *> > istack;
	QListViewItem *last = 0;
	QListViewItem *parent = 0;
	QListViewItem *item;

	// Yes, it is this ugly. Two reasons - root items must have a QListView parent,
	// rather than QListViewItem; and the TOC can move in and out an arbitrary number
	// of levels

	for (vector< Buffer::TocItem >::const_iterator iter = toclist.begin();
		iter != toclist.end(); ++iter) {
		if (iter->depth == curdepth) {
			// insert it after the last one we processed
			if (!parent)
				item = (last) ? (new QListViewItem(dialog_->tree,last)) : (new QListViewItem(dialog_->tree));
			else
				item = (last) ? (new QListViewItem(parent,last)) : (new QListViewItem(parent));
		} else if (iter->depth > curdepth) {
			int diff = iter->depth - curdepth;
			// first save old parent and last
			while (diff--)
				istack.push(pair< QListViewItem *, QListViewItem * >(parent,last));
			item = (last) ? (new QListViewItem(last)) : (new QListViewItem(dialog_->tree));
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
				item = (last) ? (new QListViewItem(dialog_->tree,last)) : (new QListViewItem(dialog_->tree));
			else
				item = (last) ? (new QListViewItem(parent,last)) : (new QListViewItem(parent));
		}
		lyxerr[Debug::GUI] << "Table of contents" << endl << "Added item " << iter->str.c_str()
			<< " at depth " << iter->depth << ", previous sibling \"" << (last ? last->text(0) : "0")
			<< "\", parent \"" << (parent ? parent->text(0) : "0") << "\"" << endl;
		item->setText(0,iter->str.c_str());
		item->setOpen(iter->depth < depth);
		curdepth = iter->depth;
		last = item;
	}

	dialog_->tree->setUpdatesEnabled(true);
	dialog_->tree->update();
}

void FormToc::setType(Buffer::TocType toctype)
{
	type = toctype;
	switch (type) {
		case Buffer::TOC_TOC:
			dialog_->setCaption(_("Table of Contents"));
			dialog_->tree->setColumnText(0,_("Table of Contents"));
			dialog_->depth->setEnabled(true);
			break;
		case Buffer::TOC_LOF:
			dialog_->setCaption(_("List of Figures"));
			dialog_->tree->setColumnText(0,_("List of Figures"));
			dialog_->depth->setEnabled(false);
			break;
		case Buffer::TOC_LOT:
			dialog_->setCaption(_("List of Tables"));
			dialog_->tree->setColumnText(0,_("List of Tables"));
			dialog_->depth->setEnabled(false);
			break;
		case Buffer::TOC_LOA:
			dialog_->setCaption(_("List of Algorithms"));
			dialog_->tree->setColumnText(0,_("List of Algorithms"));
			dialog_->depth->setEnabled(false);
			break;
	}
}

void FormToc::set_depth(int newdepth)
{
	if (newdepth!=depth)
		updateToc(newdepth);
}

// we can safely ignore the parameter because we can always update
void FormToc::update(bool)
{
	if (params.getCmdName()=="tableofcontents") {
		setType(Buffer::TOC_TOC);
		dialog_->menu->setCurrentItem(0);
	} else if (params.getCmdName()=="listoffigures") {
		setType(Buffer::TOC_LOF);
		dialog_->menu->setCurrentItem(1);
	} else if (params.getCmdName()=="listoftables") {
		setType(Buffer::TOC_LOT);
		dialog_->menu->setCurrentItem(2);
	} else {
		setType(Buffer::TOC_LOA);
		dialog_->menu->setCurrentItem(3);
	}

	updateToc(depth);
}

void FormToc::select(const char *text)
{
	if (!lv_->view()->available())
		return;

	vector <Buffer::TocItem>::const_iterator iter = toclist.begin();
	for (; iter != toclist.end(); ++iter) {
		if (iter->str==text)
			break;
	}
	
	if (iter==toclist.end()) {
		lyxerr[Debug::GUI] << "Couldn't find highlighted TOC entry : " << text << endl;
		return;
	}

	lv_->getLyXFunc()->Dispatch(LFUN_GOTO_PARAGRAPH, tostr(iter->par->id()).c_str());
}

void FormToc::set_type(Buffer::TocType toctype)
{
	if (toctype==type)
		return;

	setType(toctype);
	updateToc(depth);
}

void FormToc::show()
{
	if (!dialog_)
		dialog_ = new TocDialog(this, 0, _("LyX: Table of Contents"), false);

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormToc::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormToc::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

void FormToc::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}

void FormToc::hide()
{
	dialog_->hide();
	close();
}
