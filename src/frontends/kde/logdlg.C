/**
 * \file logdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <gettext.h>
 
#include "ControlLog.h" 
#include "FormLog.h"
#include "logdlg.h"

LogDialog::LogDialog (FormLog * f, QWidget * parent, char const  * name)
	: LogDialogData(parent, name), form_(f)
{
	line_viewer->setMinimumWidth(50);
	line_viewer->setMinimumHeight(50);
}


void LogDialog::setLogText(string const & text)
{
	setUpdatesEnabled(false);
	line_viewer->setText(text.c_str());
	setUpdatesEnabled(true);
	update();
}

void LogDialog::closePressed()
{
	form_->CancelButton();
}


void LogDialog::updatePressed()
{
	// FIXME: hmm 
	//form_->controller().setParams();
}


void LogDialog::closeEvent(QCloseEvent * e)
{
	form_->CancelButton();
	e->accept();
}
