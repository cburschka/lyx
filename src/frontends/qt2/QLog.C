/**
 * \file QLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "qt_helpers.h"
#include "ControlLog.h"
#include "support/std_sstream.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "QLogDialog.h"
#include "QLog.h"
#include "Qt2BC.h"

#include <fstream>

using std::ifstream;
using std::ostringstream;
using std::string;

typedef QController<ControlLog, QView<QLogDialog> > base_class;

QLog::QLog(Dialog & parent)
	: base_class(parent, _("LyX: LaTeX Log"))
{
}


void QLog::build_dialog()
{
	dialog_.reset(new QLogDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void QLog::update_contents()
{
	std::pair<Buffer::LogType, string> const & logfile =
		controller().logfile();

	if (logfile.first == Buffer::buildlog)
		setTitle(_("Build log"));
	else
		setTitle(_("LaTeX log"));

	dialog_->logTV->setText("");

	ifstream ifstr(logfile.second.c_str());
	if (!ifstr) {
		if (logfile.first == Buffer::buildlog)
			dialog_->logTV->setText(qt_("No build log file found."));
		else
			dialog_->logTV->setText(qt_("No LaTeX log file found."));
		return;
	}

	ostringstream ost;
	ost << ifstr.rdbuf();

	dialog_->logTV->setText(toqstr(ost.str()));
}
