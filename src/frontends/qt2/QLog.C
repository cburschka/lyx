/**
 * \file QLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "gettext.h"
#include "ControlLog.h"
#include "Lsstream.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QLogDialog.h"
#include "QLog.h"
#include "Qt2BC.h"

#include <fstream>

using std::ifstream;
using std::getline;

typedef Qt2CB<ControlLog, Qt2DB<QLogDialog> > base_class;

QLog::QLog()
	: base_class(_("Log"))
{
}


void QLog::build_dialog()
{
	dialog_.reset(new QLogDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QLog::update_contents()
{
	std::pair<Buffer::LogType, string> const & logfile =
		controller().logfile();

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

	ostringstream ost;
	ost << ifstr.rdbuf();

	dialog_->logTV->setText(ost.str().c_str());
}
