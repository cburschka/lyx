/**
 * \file QCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#include "gettext.h"
#include "QCharacterDialog.h"
#include "QCharacter.h"
#include "bufferview_funcs.h"
#include "Dialogs.h"
#include "Liason.h"
#include "QtLyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "language.h"
#include "support/lstrings.h"
#include "BufferView.h"

using SigC::slot;
using Liason::setMinibuffer;

QCharacter::QCharacter(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	d->showCharacter.connect(slot(this, &QCharacter::show));
	// for LFUN_FREE
	d->setUserFreeFont.connect(slot(this, &QCharacter::apply));
}


QCharacter::~QCharacter()
{
	delete dialog_;
}


void QCharacter::apply()
{
	if (!lv_->view()->available() || !dialog_)
		return;

	LyXFont font = dialog_->getChar();

	if (dialog_->langItem()==1)
		font.setLanguage(lv_->buffer()->params.language);

	toggleAndShow(lv_->view(), font, dialog_->toggleAll());
	lv_->view()->setState();
	lv_->buffer()->markDirty();
	setMinibuffer(lv_, _("Character set"));
}
 

void QCharacter::show()
{
	if (!dialog_) {
		dialog_ = new QCharacterDialog(this, 0, _("Character Settings"), false);
 
		// add languages
		for (Languages::const_iterator cit = languages.begin();
			cit != languages.end(); ++cit) {
			string const language = cit->second.lang();
			dialog_->lang->insertItem(tostr(language).c_str(), -1);
		}
	}

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &QCharacter::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &QCharacter::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();
	update();
	dialog_->show();
}

 
void QCharacter::close()
{
	h_.disconnect();
	u_.disconnect();
}


void QCharacter::hide()
{
	dialog_->hide();
	close();
}


void QCharacter::update(bool)
{
	if (!lv_->view()->available())
		return;

	dialog_->setReadOnly(lv_->buffer()->isReadonly());
}
