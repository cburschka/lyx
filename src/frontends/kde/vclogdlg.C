/**
 * \file vclogdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>

#include "FormVCLog.h"
#include "ControlVCLog.h"

#include "vclogdlg.h"

VCLogDialog::VCLogDialog (FormVCLog * f, QWidget * parent, char const  * name)
	: LogDialogData(parent, name), form_(f)
{
	line_viewer->setMinimumWidth(50);
	line_viewer->setMinimumHeight(50);
}


void VCLogDialog::setLogText(string const & text)
{
	setUpdatesEnabled(false);
	line_viewer->setText(text.c_str());
	setUpdatesEnabled(true);
	update();
}

void VCLogDialog::closePressed()
{
	form_->CancelButton();
}


void VCLogDialog::updatePressed()
{
	// FIXME: hmm 
	//form_->controller().setParams();
}
