/*
 * FormRef.C
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
#include "FormRef.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "formrefdialog.h"
#include "debug.h"

FormRef::FormRef(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0),
	sort(0), gotowhere(GOTOREF), type(REF), refs(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showRef.connect(slot(this, &FormRef::showRef));
	d->createRef.connect(slot(this, &FormRef::createRef));
}

FormRef::~FormRef()
{
	delete dialog_;
}

void FormRef::showRef(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	ih_ = inset_->hide.connect(slot(this,&FormRef::hide));
	params = inset->params();
	
	show();
}

void FormRef::createRef(string const & arg)
{
	if (inset_)
		close();

	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}

void FormRef::select(const char *text)
{
	highlight(text);
	goto_ref();
}

void FormRef::highlight(const char *text)
{
	if (gotowhere==GOTOBACK)
		goto_ref();

	dialog_->buttonGoto->setEnabled(true);
	if (!readonly) {
		dialog_->type->setEnabled(true);
		dialog_->reference->setText(text);
		dialog_->buttonOk->setEnabled(true);
	}
}

void FormRef::set_sort(bool on)
{
	if (on!=sort) {
		sort=on;
		dialog_->refs->clear();
		updateRefs();
	}
}

void FormRef::goto_ref()
{
	switch (gotowhere) {
		case GOTOREF:
			lv_->getLyXFunc()->Dispatch(LFUN_REF_GOTO, dialog_->reference->text());
			gotowhere=GOTOBACK;
			dialog_->buttonGoto->setText(_("&Go back"));
			break;
		case GOTOBACK:
			lv_->getLyXFunc()->Dispatch(LFUN_REF_BACK);
			gotowhere=GOTOREF;
			dialog_->buttonGoto->setText(_("&Goto reference"));
			break;
		}
}

void FormRef::updateRefs()
{
	// list will be re-done, should go back if necessary
	if (gotowhere==GOTOBACK) {
		lv_->getLyXFunc()->Dispatch(LFUN_REF_BACK);
		gotowhere = GOTOREF;
		dialog_->buttonGoto->setText(_("&Goto reference"));
	}

	dialog_->refs->setAutoUpdate(false);

	// need this because Qt will send a highlight() here for
	// the first item inserted
	string tmp(dialog_->reference->text());

	for (vector< string >::const_iterator iter = refs.begin();
		iter != refs.end(); ++iter) {
		if (sort)
			dialog_->refs->inSort(iter->c_str());
		else
			dialog_->refs->insertItem(iter->c_str());
	}

	dialog_->reference->setText(tmp.c_str());

	for (unsigned int i = 0; i < dialog_->refs->count(); ++i) {
		if (!strcmp(dialog_->reference->text(),dialog_->refs->text(i)))
			dialog_->refs->setCurrentItem(i);
	}

	dialog_->refs->setAutoUpdate(true);
	dialog_->refs->update();
}

void FormRef::do_ref_update()
{
	refs.clear();
	dialog_->refs->clear();
	refs = lv_->buffer()->getLabelList();
	if (!refs.empty())
		dialog_->sort->setEnabled(true);
	updateRefs();
}

void FormRef::update()
{
	dialog_->reference->setText(params.getContents().c_str());
	dialog_->refname->setText(params.getOptions().c_str());

	if (params.getCmdName()=="pageref") {
		type = PAGEREF;
		dialog_->type->setCurrentItem(1);
	} else if (params.getCmdName()=="vref") {
		type = VREF;
		dialog_->type->setCurrentItem(2);
	} else if (params.getCmdName()=="vpageref") {
		type = VPAGEREF;
		dialog_->type->setCurrentItem(3);
	} else if (params.getCmdName()=="prettyref") {
		type = PRETTYREF;
		dialog_->type->setCurrentItem(4);
	} else {
		type = REF;
		dialog_->type->setCurrentItem(0);
	}

	dialog_->buttonGoto->setText(_("&Goto reference"));

	gotowhere = GOTOREF;

	dialog_->sort->setChecked(sort);

	do_ref_update();

	dialog_->buttonGoto->setEnabled(params.getContents()!="");
	dialog_->buttonOk->setEnabled(params.getContents()!="");

	dialog_->type->setEnabled(!readonly);
	dialog_->sort->setEnabled(!readonly);
	dialog_->refs->setEnabled(!readonly);
	dialog_->buttonOk->setEnabled(!readonly);
	dialog_->buttonUpdate->setEnabled(!readonly);
	if (readonly)
		dialog_->buttonCancel->setText(_("&Close"));
	else
		dialog_->buttonCancel->setText(_("&Cancel"));
}

void FormRef::apply()
{
	if (readonly)
		return;

	if (!lv_->view()->available())
		return;

	switch (dialog_->type->currentItem()) {
		case 0:
			params.setCmdName("ref");
			break;
		case 1:
			params.setCmdName("pageref");
			break;
		case 2:
			params.setCmdName("vref");
			break;
		case 3:
			params.setCmdName("vpageref");
			break;
		case 4:
			params.setCmdName("prettyref");
			break;
		default:
			lyxerr[Debug::GUI] << "Unknown Ref Type" << endl;
	}

	params.setContents(dialog_->reference->text());
	params.setOptions(dialog_->refname->text());

	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->Dispatch(LFUN_REF_INSERT, params.getAsString().c_str());
}

void FormRef::show()
{
	if (!dialog_)
		dialog_ = new FormRefDialog(this, 0, _("LyX: Cross Reference"), false);

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormRef::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormRef::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

void FormRef::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}

void FormRef::hide()
{
	dialog_->hide();
	close();
}
