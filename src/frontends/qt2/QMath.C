/**
 * \file QMath.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "gettext.h"
#include "QMathDialog.h"
#include "QMathMatrixDialog.h"
#include "QDelimiterDialog.h"
#include "QMath.h"


typedef QController<ControlMath, QView<QMathDialog> > math_base;


QMath::QMath(Dialog & parent)
	: math_base(parent, _("LyX: Math Panel"))
{}


void QMath::build_dialog()
{
	dialog_.reset(new QMathDialog(this));
}


typedef QController<ControlMath, QView<QMathMatrixDialog> > matrix_base;


QMathMatrix::QMathMatrix(Dialog & parent)
	: matrix_base(parent, _("LyX: Insert Matrix"))
{}


void QMathMatrix::build_dialog()
{
	dialog_.reset(new QMathMatrixDialog(this));
}


typedef QController<ControlMath, QView<QDelimiterDialog> > delimiter_base;


QMathDelimiter::QMathDelimiter(Dialog & parent)
	: delimiter_base(parent, _("LyX: Insert Delimiter"))
{}


void QMathDelimiter::build_dialog()
{
	dialog_.reset(new QDelimiterDialog(this));
}
