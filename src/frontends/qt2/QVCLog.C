/**
 * \file QVCLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "Lsstream.h"
#include "qt_helpers.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "ControlVCLog.h"

#include "QVCLog.h"
#include "QVCLogDialog.h"
#include "Qt2BC.h"

#include <qtextview.h>
#include <qpushbutton.h>

#include "support/BoostFormat.h"

using namespace lyx::support;

typedef QController<ControlVCLog, QView<QVCLogDialog> > base_class;


QVCLog::QVCLog(Dialog & parent)
	: base_class(parent, _("LyX: Version Control Log"))
{
}


void QVCLog::build_dialog()
{
	dialog_.reset(new QVCLogDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void QVCLog::update_contents()
{
	setTitle(bformat(_("Version control log for %1$s"),
			 controller().getBufferFileName()));

	dialog_->vclogTV->setText("");

	ostringstream ss;
	controller().getVCLogFile(ss);

	dialog_->vclogTV->setText(toqstr(ss.str()));
}
