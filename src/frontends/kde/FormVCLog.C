/**
 * \file FormVCLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
#include <fstream>

#include "FormVCLog.h"
#include "ControlVCLog.h"
#include "vclogdlg.h" 
#include "gettext.h"

using std::ifstream;
using std::getline;

FormVCLog::FormVCLog(ControlVCLog & c)
	: KFormBase<ControlVCLog, VCLogDialog>(c)
{
}


void FormVCLog::update()
{
	const string logfile = controller().logfile();

	// FIXME 
	//dialog_->setCaption(string(_("Version control log for ") + controller().filename()).c_str()); 

	dialog_->setLogText("");

	ifstream ifstr(logfile.c_str());
	if (!ifstr) {
		dialog_->setLogText(_("No version control log file found"));
	//	lyx::unlink(logfile);
		return;
	}

	string text;
	string line;

	while (getline(ifstr, line))
		text += line + "\n";
 
	dialog_->setLogText(text);

	//lyx::unlink(logfile);
}


void FormVCLog::build()
{
	dialog_.reset(new VCLogDialog(this, 0, _("LyX: Version Control Log")));

	bc().setCancel(dialog_->button_cancel);
}
