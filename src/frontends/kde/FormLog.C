/**
 * \file FormLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <fstream> 

#include "Dialogs.h"
#include "FormLog.h"
#include "gettext.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "logdlg.h"
#include "lyxrc.h"

using std::ifstream;
using std::getline;

FormLog::FormLog(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	d->showLogFile.connect(slot(this, &FormLog::show));
}


FormLog::~FormLog()
{
	delete dialog_;
}


void FormLog::update()
{
	supdate();
}

 
void FormLog::supdate(bool)
{
	std::pair<Buffer::LogType, string> const logfile
		= lv_->view()->buffer()->getLogName();

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


void FormLog::show()
{
	if (!dialog_)
		dialog_ = new LogDialog(this, 0, _("LyX: LaTeX Log"));

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormLog::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormLog::supdate));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}


void FormLog::close()
{
	h_.disconnect();
	u_.disconnect();
}


void FormLog::hide()
{
	dialog_->hide();
	close();
}
