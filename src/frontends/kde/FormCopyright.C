/**
 * \file FormCopyright.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author John Levon
 */
 
#include <config.h>

#include "FormCopyright.h"
#include "ControlCopyright.h"
#include "copyrightdlg.h"
#include "gettext.h"
#include "dlg/helpers.h" 

FormCopyright::FormCopyright(ControlCopyright & c)
	: KFormBase<ControlCopyright, CopyrightDialog>(c)
{
}


void FormCopyright::build()
{
	dialog_.reset(new CopyrightDialog(this, 0, _("LyX: Copyright and Warranty")));

	// FIXME: Qt is really badly dumb in resizing these strings if we 
	// use minimum size hint :/
	dialog_->label_copyright->setText(controller().getCopyright().c_str());
	dialog_->label_licence->setText(controller().getLicence().c_str());
	dialog_->label_disclaimer->setText(controller().getDisclaimer().c_str()); 

	bc().setCancel(dialog_->button_cancel); 
}
