/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *       
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

/* FormCopyright.C
 * FormCopyright Interface Class Implementation
 */

#include <config.h>
#include "Dialogs.h"
#include "FormCopyright.h"

FormCopyright::FormCopyright(LyXView * lv, Dialogs * d)
	: dialog_(NULL), lv_(lv), d_(d), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCopyright.connect(slot(this, &FormCopyright::show));
}


FormCopyright::~FormCopyright()
{
	if (dialog_!=NULL) hide();
}


void FormCopyright::show()
{
	if(dialog_!=NULL) { // "About" box hasn't been closed, so just raise it
		Gdk_Window dialog_win(dialog_->get_window());
		dialog_win.show();
		dialog_win.raise();
	}
	else { // create new "About" dialog box
		vector<string> authors; //null. Authors are not listed in LyX copyright dialogbox.
		
		dialog_ = new Gnome::About(PACKAGE, VERSION,
					   "(C) 1995 by Matthias Ettrich, \n1995-2000 LyX Team",
					   authors,
					   "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nLyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc.,  675 Mass Ave, Cambridge, MA 02139, USA.",
					   NULL);
		
		// it should be disconnected before deleting dialog_ in FormCopyright::hide()
		destroy_ = dialog_->destroy.connect(slot(this, &FormCopyright::hide));
		
		dialog_->show();
		
		h_ = d_->hideAll.connect(slot(this, &FormCopyright::hide));
	}
}


void FormCopyright::hide()
{
	if (dialog_!=NULL) {
		dialog_->hide();
		
		h_.disconnect();
		destroy_.disconnect();
		
		delete dialog_;
		dialog_ = NULL;
	}
}
