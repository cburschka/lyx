/**
 * \file QIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QIndexDialog.h"
#include "QtLyXView.h" 
#include "BufferView.h"

#include "Dialogs.h"
#include "QIndex.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxfunc.h" 

#include <qlineedit.h>
#include <qpushbutton.h>

QIndex::QIndex(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0)
{
	d->showIndex.connect(slot(this, &QIndex::showIndex));
	d->createIndex.connect(slot(this, &QIndex::createIndex));
}


QIndex::~QIndex()
{
	delete dialog_;
}


void QIndex::showIndex(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	//FIXME ih_ = inset_->hide.connect(slot(this,&QIndex::hide));
	params = inset->params();
	
	show();
}

 
void QIndex::createIndex(string const & arg)
{
	// we could already be showing a URL, clear it out
	if (inset_)
		close();
 
	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}

 
void QIndex::update()
{
	dialog_->keywordED->setText(params.getContents().c_str());

	if (readonly) {
		dialog_->keywordED->setFocusPolicy(QWidget::NoFocus);
		dialog_->okPB->setEnabled(false);
		dialog_->cancelPB->setText(_("Close"));
	} else {
		dialog_->keywordED->setFocusPolicy(QWidget::StrongFocus);
		dialog_->keywordED->setFocus();
		dialog_->okPB->setEnabled(true);
		dialog_->cancelPB->setText(_("Cancel"));
	}
}

 
void QIndex::apply()
{
	if (readonly)
		return;

	params.setContents(dialog_->keywordED->text().latin1());

	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->dispatch(LFUN_INDEX_INSERT, params.getAsString().c_str());
}

 
void QIndex::show()
{
	if (!dialog_)
		dialog_ = new QIndexDialog(this, 0, _("LyX: Index"), false);
 
	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &QIndex::hide));
		//u_ = d_->updateBufferDependent.connect(slot(this, &QIndex::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();
 
	update();
	dialog_->show();
}


void QIndex::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}

 
void QIndex::hide()
{
	dialog_->hide();
	close();
}
