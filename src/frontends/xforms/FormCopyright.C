/* FormCopyright.C
 * FormCopyright Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "LyXView.h"
#include "form_copyright.h"
#include "FormCopyright.h"
#include "xform_helpers.h"

FormCopyright::FormCopyright( LyXView * lv, Dialogs * d	)
	: FormBaseBI(lv, d, _("Copyright and Warranty"), new OkCancelPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}


FormCopyright::~FormCopyright()
{
	delete dialog_;
}


FL_FORM * FormCopyright::form() const
{
	if (dialog_ ) return dialog_->form;
	return 0;
}


void FormCopyright::build()
{
	dialog_ = build_copyright();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	string str = _("LyX is Copyright (C) 1995 by Matthias Ettrich, 1995-2001 LyX Team");
	str = formatted(str, dialog_->text_copyright->w-10,
			FL_NORMAL_SIZE, FL_NORMAL_STYLE);
	fl_set_object_label(dialog_->text_copyright, str.c_str());

	str = _("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.");
	str = formatted(str, dialog_->text_licence->w-10,
			FL_NORMAL_SIZE, FL_NORMAL_STYLE);
	fl_set_object_label(dialog_->text_licence, str.c_str());

	str = _("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");
	str = formatted(str, dialog_->text_disclaimer->w-10,
			FL_NORMAL_SIZE, FL_NORMAL_STYLE);
	fl_set_object_label(dialog_->text_disclaimer, str.c_str());
	
        // Manage the cancel/close button
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();
}
