/**
 * \file QToc.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include <stack>

#include <qslider.h>
#include "QTocDialog.h"
#include "QtLyXView.h"

#include "Dialogs.h"
#include "BufferView.h" 
#include "QToc.h"
#include "gettext.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "lyxfunc.h"
#include "debug.h"

using std::vector;
using std::pair;
using std::stack;
using std::endl;

#include <qlistview.h>
#include <qcombobox.h>
 
QToc::QToc(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0),
	/*toclist(0),*/ depth(1)
{
	d->showTOC.connect(slot(this, &QToc::showTOC));
	d->createTOC.connect(slot(this, &QToc::createTOC));
}

 
QToc::~QToc()
{
	delete dialog_;
}


void QToc::showTOC(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset == 0)
		return;

	inset_ = inset;
	//FIXME ih_ = inset_->hide.connect(slot(this,&QToc::hide));
	params = inset->params();
	
	show();
}


void QToc::createTOC(string const & arg)
{
	if (inset_)
		close();

	params.setFromString(arg);
	show();
}


void QToc::updateToc(int newdepth)
{
	if (!lv_->view()->available()) {
		//toclist.clear();
		dialog_->tocLV->clear();
		return;
	}

#if 0 
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

	dialog_->tocLV->clear();
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

	for (vector< Buffer::TocItem >::const_iterator iter = toclist.begin();
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
		item->setOpen(iter->depth < depth);
		curdepth = iter->depth;
		last = item;
	}

	dialog_->tocLV->setUpdatesEnabled(true);
	dialog_->tocLV->update();
}


void QToc::setType(Buffer::TocType toctype)
{
	type = toctype;
	switch (type) {
		case Buffer::TOC_TOC:
			dialog_->setCaption(_("Table of Contents"));
			dialog_->tocLV->setColumnText(0,_("Table of Contents"));
			dialog_->depthSL->setEnabled(true);
			break;
		case Buffer::TOC_LOF:
			dialog_->setCaption(_("List of Figures"));
			dialog_->tocLV->setColumnText(0,_("List of Figures"));
			dialog_->depthSL->setEnabled(false);
			break;
		case Buffer::TOC_LOT:
			dialog_->setCaption(_("List of Tables"));
			dialog_->tocLV->setColumnText(0,_("List of Tables"));
			dialog_->depthSL->setEnabled(false);
			break;
		case Buffer::TOC_LOA:
			dialog_->setCaption(_("List of Algorithms"));
			dialog_->tocLV->setColumnText(0,_("List of Algorithms"));
			dialog_->depthSL->setEnabled(false);
			break;
	}
}


void QToc::set_depth(int newdepth)
{
	if (newdepth!=depth)
		updateToc(newdepth);
#endif 
}


void QToc::update()
{
#if 0 
	if (params.getCmdName()=="tableofcontents") {
		setType(Buffer::TOC_TOC);
		dialog_->typeCO->setCurrentItem(0);
	} else if (params.getCmdName()=="listoffigures") {
		setType(Buffer::TOC_LOF);
		dialog_->typeCO->setCurrentItem(1);
	} else if (params.getCmdName()=="listoftables") {
		setType(Buffer::TOC_LOT);
		dialog_->typeCO->setCurrentItem(2);
	} else {
		setType(Buffer::TOC_LOA);
		dialog_->typeCO->setCurrentItem(3);
	}

	updateToc(depth);
}


void QToc::select(const char *text)
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


void QToc::set_type(Buffer::TocType toctype)
{
	if (toctype==type)
		return;

	setType(toctype);
	updateToc(depth);
#endif 
}


void QToc::show()
{
	if (!dialog_)
		dialog_ = new QTocDialog(this, 0, _("LyX: Table of Contents"), false);

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &QToc::hide));
		//u_ = d_->updateBufferDependent.connect(slot(this, &QToc::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}


void QToc::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}


void QToc::hide()
{
	dialog_->hide();
	close();
}
