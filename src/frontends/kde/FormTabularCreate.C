/*
 * FormTabularCreate.C
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

#include "tabcreatedlg.h"
#include "Dialogs.h"
#include "FormTabularCreate.h"
#include "gettext.h"
#include "QtLyXView.h"
#include "BufferView.h"
#include "insets/insettabular.h"  

FormTabularCreate::FormTabularCreate(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTabularCreate.connect(slot(this, &FormTabularCreate::show));
}

FormTabularCreate::~FormTabularCreate()
{
	delete dialog_;
}

void FormTabularCreate::apply(unsigned int rows, unsigned cols)
{
	if (!lv_->view()->available())
		return;

	InsetTabular * in = new InsetTabular(*lv_->buffer(), rows, cols);
	if (!lv_->view()->open_new_inset(in))
		delete in;
}

void FormTabularCreate::show()
{
	if (!dialog_)
		dialog_ = new TabularCreateDialog(this, 0, _("LyX: Insert Table"));

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormTabularCreate::hide));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

void FormTabularCreate::close()
{
	h_.disconnect();
}

void FormTabularCreate::hide()
{
	dialog_->hide();
	close();
}
