/**
 * \file QRef.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "QRefDialog.h"

#include "QtLyXView.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "QRef.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxfunc.h"
#include "debug.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

using std::endl;

QRef::QRef(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0),
	sort(0), gotowhere(GOTOREF), type(REF), refs(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showRef.connect(slot(this, &QRef::showRef));
	d->createRef.connect(slot(this, &QRef::createRef));
}


QRef::~QRef()
{
	delete dialog_;
}


void QRef::showRef(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	//ih_ = inset_->hide.connect(slot(this,&QRef::hide));
	params = inset->params();
	
	show();
}


void QRef::createRef(string const & arg)
{
	if (inset_)
		close();

	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}


void QRef::select(const char *text)
{
	highlight(text);
	goto_ref();
}

 
void QRef::highlight(const char *text)
{
	if (gotowhere==GOTOBACK)
		goto_ref();

	dialog_->gotoPB->setEnabled(true);
	if (!readonly) {
		dialog_->typeCO->setEnabled(true);
		dialog_->referenceED->setText(text);
		dialog_->okPB->setEnabled(true);
	}
}

 
void QRef::set_sort(bool on)
{
	if (on!=sort) {
		sort=on;
		dialog_->refsLB->clear();
		updateRefs();
	}
}

 
void QRef::goto_ref()
{
	switch (gotowhere) {
		case GOTOREF:
			lv_->getLyXFunc()->dispatch(LFUN_REF_GOTO, dialog_->referenceED->text().latin1());
			gotowhere=GOTOBACK;
			dialog_->gotoPB->setText(_("&Go back"));
			break;
		case GOTOBACK:
			//FIXME lv_->getLyXFunc()->dispatch(LFUN_REF_BACK);
			gotowhere=GOTOREF;
			dialog_->gotoPB->setText(_("&Goto reference"));
			break;
		}
}

 
void QRef::updateRefs()
{
	// list will be re-done, should go back if necessary
	if (gotowhere==GOTOBACK) {
		//FIXME lv_->getLyXFunc()->dispatch(LFUN_REF_BACK);
		gotowhere = GOTOREF;
		dialog_->gotoPB->setText(_("&Goto reference"));
	}

	dialog_->refsLB->setAutoUpdate(false);

	// need this because Qt will send a highlight() here for
	// the first item inserted
	string tmp(dialog_->referenceED->text());

	for (std::vector< string >::const_iterator iter = refs.begin();
		iter != refs.end(); ++iter) {
		if (sort)
			dialog_->refsLB->inSort(iter->c_str());
		else
			dialog_->refsLB->insertItem(iter->c_str());
	}

	dialog_->referenceED->setText(tmp.c_str());

	for (unsigned int i = 0; i < dialog_->refsLB->count(); ++i) {
		if (!strcmp(dialog_->referenceED->text(),dialog_->refsLB->text(i)))
			dialog_->refsLB->setCurrentItem(i);
	}

	dialog_->refsLB->setAutoUpdate(true);
	dialog_->refsLB->update();
}

 
void QRef::do_ref_update()
{
	refs.clear();
	dialog_->refsLB->clear();
	refs = lv_->buffer()->getLabelList();
	if (!refs.empty())
		dialog_->sortCB->setEnabled(true);
	updateRefs();
}
 

void QRef::update()
{
	dialog_->referenceED->setText(params.getContents().c_str());
	dialog_->nameED->setText(params.getOptions().c_str());

	if (params.getCmdName()=="pageref") {
		type = PAGEREF;
		dialog_->typeCO->setCurrentItem(1);
	} else if (params.getCmdName()=="vref") {
		type = VREF;
		dialog_->typeCO->setCurrentItem(2);
	} else if (params.getCmdName()=="vpageref") {
		type = VPAGEREF;
		dialog_->typeCO->setCurrentItem(3);
	} else if (params.getCmdName()=="prettyref") {
		type = PRETTYREF;
		dialog_->typeCO->setCurrentItem(4);
	} else {
		type = REF;
		dialog_->typeCO->setCurrentItem(0);
	}

	dialog_->gotoPB->setText(_("&Goto reference"));

	gotowhere = GOTOREF;

	dialog_->sortCB->setChecked(sort);

	do_ref_update();

	dialog_->gotoPB->setEnabled(params.getContents()!="");
	dialog_->okPB->setEnabled(params.getContents()!="");

	dialog_->typeCO->setEnabled(!readonly);
	dialog_->sortCB->setEnabled(!readonly);
	dialog_->refsLB->setEnabled(!readonly);
	dialog_->okPB->setEnabled(!readonly);
	dialog_->updatePB->setEnabled(!readonly);
	if (readonly)
		dialog_->cancelPB->setText(_("&Close"));
	else
		dialog_->cancelPB->setText(_("&Cancel"));
}

 
void QRef::apply()
{
	if (readonly)
		return;

	if (!lv_->view()->available())
		return;

	switch (dialog_->typeCO->currentItem()) {
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

	params.setContents(dialog_->referenceED->text().latin1());
	params.setOptions(dialog_->nameED->text().latin1());

	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->dispatch(LFUN_REF_INSERT, params.getAsString().c_str());
}

 
void QRef::show()
{
	if (!dialog_)
		dialog_ = new QRefDialog(this, 0, _("LyX: Cross Reference"), false);

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &QRef::hide));
		//u_ = d_->updateBufferDependent.connect(slot(this, &QRef::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

 
void QRef::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}

 
void QRef::hide()
{
	dialog_->hide();
	close();
}
