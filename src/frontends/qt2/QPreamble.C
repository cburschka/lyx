/**
 * \file QPreamble.C
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

#include "qt_helpers.h"
#include "LyXView.h"
#include "ControlPreamble.h"

#include "QPreamble.h"
#include "QPreambleDialog.h"
#include "Qt2BC.h"

#include <qmultilineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlPreamble, Qt2DB<QPreambleDialog> > base_class;


QPreamble::QPreamble()
	: base_class(qt_("LaTeX Preamble"))
{
}


void QPreamble::build_dialog()
{
	dialog_.reset(new QPreambleDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
}


void QPreamble::update_contents()
{
	dialog_->preambleLE->setText(toqstr(controller().params()));
}


void QPreamble::apply()
{
	controller().params(fromqstr(dialog_->preambleLE->text()));
}
