/**
 * \file QLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "gettext.h"
#include "ControlLog.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QLogDialog.h"
#include "QLog.h"
#include "Qt2BC.h"

using std::ifstream;
using std::getline;

typedef Qt2CB<ControlLog, Qt2DB<QLogDialog> > base_class;

QLog::QLog(ControlLog & c, Dialogs &)
	: base_class(c, _("Log"))
{
}


void QLog::build_dialog()
{
	dialog_.reset(new QLogDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QLog::update_contents()
{
	std::pair<Buffer::LogType, string> const logfile = controller().logfile();

	if (logfile.first == Buffer::buildlog)
		dialog_->setCaption(_("Build log"));
	else
		dialog_->setCaption(_("LaTeX log"));

	dialog_->logTV->setText("");

	ifstream ifstr(logfile.second.c_str());
	if (!ifstr) {
		if (logfile.first == Buffer::buildlog)
			dialog_->logTV->setText(_("No build log file found"));
		else
			dialog_->logTV->setText(_("No LaTeX log file found"));
		return;
	}

	string text;
	string line;

	while (getline(ifstr, line))
		text += line + "\n";

	dialog_->logTV->setText(text.c_str());
}
