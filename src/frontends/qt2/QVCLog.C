/**
 * \file QVCLog.C
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

#include "Lsstream.h"
#include "gettext.h"
#include "LyXView.h"
#include "ControlVCLog.h"

#include "QVCLog.h"
#include "QVCLogDialog.h"
#include "Qt2BC.h"

#include <qtextview.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlVCLog, Qt2DB<QVCLogDialog> > base_class;


QVCLog::QVCLog()
	: base_class(_("VCLog"))
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

	ostringstream ss;
	controller().getVCLogFile(ss);

	dialog_->vclogTV->setText(ss.str().c_str());
}
