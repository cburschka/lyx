/*
 * FormUrl.C
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
#include "FormUrl.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h" 
#include "formurldialog.h"

FormUrl::FormUrl(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showUrl.connect(slot(this, &FormUrl::showUrl));
	d->createUrl.connect(slot(this, &FormUrl::createUrl));
}

FormUrl::~FormUrl()
{
	delete dialog_;
}

void FormUrl::showUrl(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	ih_ = inset_->hide.connect(slot(this,&FormUrl::hide));
	params = inset->params();
	
	show();
}
 
void FormUrl::createUrl(string const & arg)
{
	// we could already be showing a URL, clear it out
	if (inset_)
		close();
 
	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}
 
void FormUrl::update(bool switched)
{
	if (switched) {
		// I suspect a buffer switch should cause hide() here. ARRae
		hide();
		return;
	}

	dialog_->url->setText(params.getContents().c_str());
	dialog_->urlname->setText(params.getOptions().c_str());

	if (params.getCmdName()=="url") 
		dialog_->htmlurl->setChecked(0);
	else
		dialog_->htmlurl->setChecked(1);

	if (readonly) {
		dialog_->urlname->setFocusPolicy(QWidget::NoFocus);
		dialog_->url->setFocusPolicy(QWidget::NoFocus);
		dialog_->buttonOk->setEnabled(false);
		dialog_->buttonCancel->setText(_("Close"));
		dialog_->htmlurl->setEnabled(false);
	} else {
		dialog_->urlname->setFocusPolicy(QWidget::StrongFocus);
		dialog_->url->setFocusPolicy(QWidget::StrongFocus);
		dialog_->url->setFocus();
		dialog_->buttonOk->setEnabled(true);
		dialog_->buttonCancel->setText(_("Cancel"));
		dialog_->htmlurl->setEnabled(true);
	}
}
 
void FormUrl::apply()
{
	if (readonly)
		return;

	params.setContents(dialog_->url->text());
	params.setOptions(dialog_->urlname->text());

	if (dialog_->htmlurl->isChecked())
		params.setCmdName("htmlurl");
	else
		params.setCmdName("url");

	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->Dispatch(LFUN_INSERT_URL, params.getAsString().c_str());
}
 
void FormUrl::show()
{
	if (!dialog_)
		dialog_ = new FormUrlDialog(this, 0, _("LyX: Url"), false);
 
	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormUrl::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormUrl::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();
 
	update();
	dialog_->show();
}

void FormUrl::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}
 
void FormUrl::hide()
{
	dialog_->hide();
	close();
}
