/*
 * FormIndex.C
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

#include "Dialogs.h"
#include "FormIndex.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h" 
#include "formindexdialog.h"

FormIndex::FormIndex(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showIndex.connect(slot(this, &FormIndex::showIndex));
	d->createIndex.connect(slot(this, &FormIndex::createIndex));
}

FormIndex::~FormIndex()
{
	delete dialog_;
}

void FormIndex::showIndex(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	ih_ = inset_->hide.connect(slot(this,&FormIndex::hide));
	params = inset->params();
	
	show();
}
 
void FormIndex::createIndex(string const & arg)
{
	// we could already be showing a URL, clear it out
	if (inset_)
		close();
 
	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}
 
void FormIndex::update()
{
	dialog_->index->setText(params.getContents().c_str());

	if (readonly) {
		dialog_->index->setFocusPolicy(QWidget::NoFocus);
		dialog_->buttonOk->setEnabled(false);
		dialog_->buttonCancel->setText(_("Close"));
	} else {
		dialog_->index->setFocusPolicy(QWidget::StrongFocus);
		dialog_->index->setFocus();
		dialog_->buttonOk->setEnabled(true);
		dialog_->buttonCancel->setText(_("Cancel"));
	}
}
 
void FormIndex::apply()
{
	if (readonly)
		return;

	params.setContents(dialog_->index->text());

	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->Dispatch(LFUN_INDEX_INSERT, params.getAsString().c_str());
}
 
void FormIndex::show()
{
	if (!dialog_)
		dialog_ = new FormIndexDialog(this, 0, _("LyX: Index"), false);
 
	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormIndex::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormIndex::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();
 
	update();
	dialog_->show();
}

void FormIndex::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}
 
void FormIndex::hide()
{
	dialog_->hide();
	close();
}
