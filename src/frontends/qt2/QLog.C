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

#include "QLog.h"
#include "QLogDialog.h"
#include "qt_helpers.h"

#include "controllers/ControlLog.h"

#include "support/std_sstream.h"

#include <qtextview.h>
#include <qpushbutton.h>

namespace lyx {
namespace frontend {

typedef QController<ControlLog, QView<QLogDialog> > base_class;

QLog::QLog(Dialog & parent)
	: base_class(parent, "")
{}


void QLog::build_dialog()
{
	dialog_.reset(new QLogDialog(this));
}


void QLog::update_contents()
{
	setTitle(controller().title());

	std::ostringstream ss;
	controller().getContents(ss);

	dialog_->logTV->setText(toqstr(ss.str()));
}

} // namespace frontend
} // namespace lyx
