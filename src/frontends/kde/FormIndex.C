/**
 * \file FormIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "Dialogs.h"
#include "FormIndex.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h" 
#include "indexdlg.h"

FormIndex::FormIndex(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0)
{
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
	// we could already be showing an index entry, clear it out
	if (inset_)
		close();
 
	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}

 
void FormIndex::update(bool switched)
{
	if (switched) {
		hide();
		return;
	}

	dialog_->setIndexText(params.getContents().c_str());
	dialog_->setReadOnly(readonly);
}

 
void FormIndex::apply()
{
	if (readonly)
		return;

	params.setContents(dialog_->getIndexText());

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
		dialog_ = new IndexDialog(this, 0, _("LyX: Index"));
 
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
