/**
 * \file QMath.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "gettext.h"
#include "ControlMath2.h"
#include "QMathDialog.h"
#include "QMath.h"


typedef QController<ControlMath2, QView<QMathDialog> > base_class;


QMath::QMath(Dialog & parent)
	: base_class(parent, _("LyX: Math Panel"))
{}


void QMath::build_dialog()
{
	dialog_.reset(new QMathDialog(this));
}
