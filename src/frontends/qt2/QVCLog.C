/**
 * \file QVCLog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include "Lsstream.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include <qtextview.h>
#include <qpushbutton.h>

#include "QVCLogDialog.h"
#include "QVCLog.h"
#include "Qt2BC.h"
#include "gettext.h"

#include "QtLyXView.h"
#include "ControlVCLog.h"

typedef Qt2CB<ControlVCLog, Qt2DB<QVCLogDialog> > base_class;

QVCLog::QVCLog(ControlVCLog & c)
	: base_class(c, _("VCLog"))
{
}


void QVCLog::build_dialog()
{
	dialog_.reset(new QVCLogDialog(this));

	bc().setCancel(dialog_->closePB);
}


void QVCLog::update_contents()
{
	dialog_->setCaption(string(_("Version control log for ") + controller().getBufferFileName()).c_str());

	dialog_->vclogTV->setText("");

	stringstream ss;

	dialog_->vclogTV->setText(controller().getVCLogFile(ss).str().c_str());
}
