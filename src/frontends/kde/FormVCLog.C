/**
 * \file FormVCLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <fstream> 

#include "Dialogs.h"
#include "FormVCLog.h"
#include "gettext.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "logdlg.h"
#include "lyxvc.h"

using std::ifstream;
using std::getline;

FormVCLog::FormVCLog(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), h_(0), u_(0)
{
	d->showVCLogFile.connect(slot(this, &FormVCLog::show));
}


FormVCLog::~FormVCLog()
{
	delete dialog_;
}


void FormVCLog::update()
{
	supdate();
}

 
void FormVCLog::supdate(bool)
{
	const string logfile = lv_->view()->buffer()->lyxvc.getLogFile();

	dialog_->setCaption(string(_("Version control log for ") + lv_->view()->buffer()->fileName()).c_str());

	dialog_->setLogText("");

	ifstream ifstr(logfile.c_str());
	if (!ifstr) {
		dialog_->setLogText(_("No version control log file found"));
		lyx::unlink(logfile);
		return;
	}

	string text;
	string line;

	while (getline(ifstr, line))
		text += line + "\n";
 
	dialog_->setLogText(text);

	lyx::unlink(logfile);
}


void FormVCLog::show()
{
	if (!dialog_)
		dialog_ = new LogDialog(this, 0, _("LyX: Version Control Log"));

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormVCLog::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormVCLog::supdate));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}


void FormVCLog::close()
{
	h_.disconnect();
	u_.disconnect();
}


void FormVCLog::hide()
{
	dialog_->hide();
	close();
}
