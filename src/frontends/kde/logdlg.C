/**
 * \file logdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include "FormLog.h"

#include "logdlg.h"

#include <config.h>

#include "debug.h" 
#include <gettext.h>

LogDialog::LogDialog (DialogBase * f, QWidget * parent, char const  * name)
	: LogDialogData(parent, name), form_(f)
{
	viewer->setMinimumWidth(50);
	viewer->setMinimumHeight(50);
}


LogDialog::~LogDialog()
{
}


void LogDialog::setLogText(string const & text)
{
	setUpdatesEnabled(false);
	viewer->setText(text.c_str());
	setUpdatesEnabled(true);
	update();
}

void LogDialog::closePressed()
{
	form_->close();
	hide();
}


void LogDialog::updatePressed()
{
	form_->update();
}
