/**
 * \file QSearch.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "gettext.h"
#include "QSearchDialog.h"
#include "QSearch.h"
#include "Dialogs.h"
#include "Liason.h"
#include "QtLyXView.h"
#include "buffer.h"
#include "lyxfind.h"
#include "support/lstrings.h"
#include "BufferView.h"

using Liason::setMinibuffer;

QSearch::QSearch(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	d->showSearch.connect(SigC::slot(this, &QSearch::show));
	// perhaps in the future we'd like a
	// "search again" button/keybinding
	// d->searchAgain.connect(slot(this, &QSearch::FindNext));
}


QSearch::~QSearch()
{
	delete dialog_;
}

 
void QSearch::show()
{
	if (!dialog_) {
		dialog_ = new QSearchDialog(this, 0, _("Find and Replace"), false);
	}

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(SigC::slot(this, &QSearch::hide));
		u_ = d_->updateBufferDependent.connect(SigC::slot(this, &QSearch::update));
	}
	
	dialog_->raise();
	dialog_->setActiveWindow();
	update();
	dialog_->show();
}

 
void QSearch::find(string const & searchstr, bool const & casesensitive,
				bool const & matchword, bool const & searchback)
{
	bool const found = LyXFind(lv_->view(), searchstr, searchback,
		casesensitive, matchword);
	
	if (!found)
		setMinibuffer(lv_, _("String not found!"));
}

 
void QSearch::replace(string const & searchstr, string const & replacestr,
			 bool const & casesensitive, bool const & matchword, 
			 bool const & searchback, bool const & replaceall)
{
	int replace_count = LyXReplace(lv_->view(), searchstr, replacestr,
				  searchback, casesensitive, matchword,
				  replaceall);
				  
	if (replace_count == 0) {
		setMinibuffer(lv_, _("String not found!"));
	} else {
		if (replace_count == 1) {
			setMinibuffer(lv_, _("String has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			setMinibuffer(lv_, str.c_str());
		}
	}
}


void QSearch::close()
{
	h_.disconnect();
	u_.disconnect();
}

 
void QSearch::hide()
{
	dialog_->hide();
	close();
}

 
void QSearch::update(bool)
{
	if (!lv_->view()->available())
		return;

	dialog_->setReadOnly(lv_->buffer()->isReadonly());
}
