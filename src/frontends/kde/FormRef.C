/**
 * \file FormRef.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "Dialogs.h"
#include "FormRef.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "refdlg.h"
#include "debug.h"
#include "insets/insetref.h"

#include <qtooltip.h>

using std::endl;

FormRef::FormRef(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0),
	sort(0), gotowhere(GOTOREF), refs(0)
{
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


void FormRef::select(char const * text)
{
	highlight(text);
	goto_ref();
}


void FormRef::highlight(char const * text)
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
	if (on != sort) {
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
			QToolTip::remove(dialog_->buttonGoto); 
			QToolTip::add(dialog_->buttonGoto,_("Jump back to original position"));
			break;
		case GOTOBACK:
			lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_GOTO, "0");
			gotowhere=GOTOREF;
			dialog_->buttonGoto->setText(_("&Goto reference"));
			QToolTip::remove(dialog_->buttonGoto); 
			QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));
			break;
		}
}


void FormRef::updateRefs()
{
	// list will be re-done, should go back if necessary
	if (gotowhere == GOTOBACK) {
		lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_GOTO, "0");
		gotowhere = GOTOREF;
		dialog_->buttonGoto->setText(_("&Goto reference"));
		QToolTip::remove(dialog_->buttonGoto); 
		QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));
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

	for (unsigned int i=0; i < dialog_->refs->count(); ++i) {
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


void FormRef::update(bool switched)
{
	if (switched) {
		hide();
		return;
	}

	dialog_->reference->setText(params.getContents().c_str());
	dialog_->refname->setText(params.getOptions().c_str());

	if (inset_)
		dialog_->type->setCurrentItem(InsetRef::getType(params.getCmdName()));

	lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_SAVE, "0");
	dialog_->buttonGoto->setText(_("&Goto reference"));
	QToolTip::remove(dialog_->buttonGoto); 
	QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));

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

	params.setCmdName(InsetRef::getName(dialog_->type->currentItem()));
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
		dialog_ = new RefDialog(this, 0, _("LyX: Cross Reference"), false);

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
