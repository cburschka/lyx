/**
 * \file FormLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <fstream>

#include "FormLog.h"
#include "ControlLog.h"
#include "logdlg.h" 
#include "gettext.h"

using std::ifstream;
using std::getline;

FormLog::FormLog(ControlLog & c)
	: KFormBase<ControlLog, LogDialog>(c)
{
}


void FormLog::update()
{
	std::pair<Buffer::LogType, string> const logfile = controller().logfile();

	if (logfile.first == Buffer::buildlog)
		dialog_->setCaption(_("Build log"));
	else
		dialog_->setCaption(_("LaTeX log"));

	dialog_->setLogText("");

	ifstream ifstr(logfile.second.c_str());
	if (!ifstr) {
		if (logfile.first == Buffer::buildlog)
			dialog_->setLogText(_("No build log file found"));
		else
			dialog_->setLogText(_("No LaTeX log file found"));
		return;
	}

	string text;
	string line;

	while (getline(ifstr, line))
		text += line + "\n";
 
	dialog_->setLogText(text);
}


void FormLog::build()
{
	dialog_.reset(new LogDialog(this, 0, _("LyX: LaTeX Log")));

	bc().setCancel(dialog_->button_cancel);
}
