/**
 * \file QTabularCreate.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QTabularCreateDialog.h"
#include "Dialogs.h"
#include "QTabularCreate.h"
#include "gettext.h"
#include "QtLyXView.h"
#include "lyxfunc.h"
#include "BufferView.h"
#include "insets/insettabular.h"
#include "support/lstrings.h"

using SigC::slot;

QTabularCreate::QTabularCreate(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	d->showTabularCreate.connect(slot(this, &QTabularCreate::show));
}


QTabularCreate::~QTabularCreate()
{
	delete dialog_;
}

 
void QTabularCreate::apply(int rows, int cols)
{
	if (!lv_->view()->available())
		return;

	string tmp = tostr(rows) + " " + tostr(cols);
	lv_->getLyXFunc()->dispatch(LFUN_INSET_TABULAR, tmp);
}

 
void QTabularCreate::show()
{
	if (!dialog_)
		dialog_ = new QTabularCreateDialog(this, 0, _("LyX: Insert Table"));

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &QTabularCreate::hide));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

 
void QTabularCreate::close()
{
	h_.disconnect();
}

 
void QTabularCreate::hide()
{
	dialog_->hide();
	close();
}
